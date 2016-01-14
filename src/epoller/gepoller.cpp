#include <epoller/gepoller.h>
#include <cstdio>
#include <cstring>
#include <iostream>

#define DBG_PREFIX "gepoller: "

int gepoller_event::handler(struct epoller *epoller, struct epoll_event *revent)
{
	if (!gfd) {
		std::cerr << DBG_PREFIX"null glib file descriptor" << std::endl;
		return false;
	}

	gfd->revents = events_epoll2glib(revent->events);

	return 0;
}

bool gepoller_event::add_to_epoller(struct epoller *epoller)
{
	if (!gfd) {
		std::cerr << DBG_PREFIX"null glib file descriptor" << std::endl;
		return false;
	}

	memset(&event, 0, sizeof event);
	event.data.ptr = this;
	event.events = events_glib2epoll(gfd->events);
	int ret = epoll_ctl(epoller->fd, EPOLL_CTL_ADD, gfd->fd, &event);
	if (ret == -1) {
		perror(DBG_PREFIX"adding file descriptor to parent epoller failed");
		return false;
	}

	return true;
}

bool gepoller_event::del_from_epoller(struct epoller *epoller)
{
	if (!gfd) {
		std::cerr << DBG_PREFIX"null glib file descriptor" << std::endl;
		return false;
	}

	int ret = epoll_ctl(epoller->fd, EPOLL_CTL_DEL, gfd->fd, NULL);
	if (ret == -1) {
		perror(DBG_PREFIX"removing file descriptor from parent epoller failed");
		return false;
	}

	return true;
}

int gepoller_event::events_glib2epoll(int events)
{
#ifdef GEPOLLER_EVENTS_COMPATIBLE
	return events;
#else
	int ev = 0;
	if (events & G_IO_IN)
		ev |= EPOLLIN;
	if (events & G_IO_OUT)
		ev |= EPOLLOUT;
	if (events & G_IO_PRI)
		ev |= EPOLLPRI;
	if (events & G_IO_ERR)
		ev |= EPOLLERR;
	if (events & G_IO_HUP)
		ev |= EPOLLHUP;
	return ev;
#endif
}

int gepoller_event::events_epoll2glib(int events)
{
#ifdef GEPOLLER_EVENTS_COMPATIBLE
	return events;
#else
	int ev = 0;
	if (events & EPOLLIN)
		ev |= G_IO_IN;
	if (events & EPOLLOUT)
		ev |= G_IO_OUT;
	if (events & EPOLLPRI)
		ev |= G_IO_PRI;
	if (events & EPOLLERR)
		ev |= G_IO_ERR;
	if (events & EPOLLHUP)
		ev |= G_IO_HUP;
	return ev;
#endif
}

bool gepoller::loop()
{
	int r, ret, to, g_priority, g_timeout, gfds_n;
	bool gevents_added = false;

	loop_exit = 0;
	while (!loop_exit) {

		// glib stuff
		if (!g_main_context_acquire(context)) {
			std::cerr << DBG_PREFIX"acquiring glib context failed" << std::endl;
			loop_exit = -1;
			break;
		}
		if (g_main_context_prepare(context, &g_priority))
			g_main_context_dispatch(context);
		gfds_n = g_main_context_query(context, g_priority, &g_timeout, gfds, gevents_size);
		g_main_context_release(context);
		if (gfds_n > (int) gevents_size) {
			std::cerr << DBG_PREFIX"not enough place to store glib file descriptors" << std::endl;
			loop_exit = -1;
			break;
		}
		if (!add_gevents_to_epoller(gfds_n)) {
			std::cerr << DBG_PREFIX"adding glib events to epoller failed" << std::endl;
			loop_exit = -1;
			break;
		}
		gevents_added = true;

		// call pre-epoll handler
		if (pre_epoll_handler) {
			r = pre_epoll_handler(this);
			if (r > 0) {
				loop_exit = 1;
				break;
			} else if (r < 0) {
				std::cerr << DBG_PREFIX"pre_epoll_handler announces exit with error" << std::endl;
				loop_exit = -1;
				break;
			}
		}

		// compute epoll_wait timeout
		if (timeout < 0 && g_timeout < 0)
			to = -1;
		else if (timeout < 0)
			to = g_timeout;
		else if (g_timeout < 0)
			to = timeout;
		else
			to = std::min(timeout, g_timeout);

		// call epoll wait
		ret = epoll_wait(fd, revents, revents_size, to);

		// call post-epoll handler
		if (post_epoll_handler) {
			r = post_epoll_handler(this);
			if (r > 0) {
				loop_exit = 1;
				break;
			} else if (r < 0) {
				std::cerr << DBG_PREFIX"post_epoll_handler announces exit with error" << std::endl;
				loop_exit = -1;
				break;
			}
		}

		if (ret == -1) {

			// error
			perror(DBG_PREFIX"epoll waiting failed");
			loop_exit = -1;
			break;

		} else if (ret == 0) {

			// call timeout handler
			if (timeout_handler) {
				r = timeout_handler(this);
				if (r > 0) {
					loop_exit = 1;
					break;
				} else if (r < 0) {
					std::cerr << DBG_PREFIX"timeout_handler announces exit with error" << std::endl;
					loop_exit = -1;
					break;
				}
			}

		} else {

			// call revents handler
			if (revents_handler) {
				r = revents_handler(this, revents, ret);
				if (r > 0) {
					loop_exit = 1;
					break;
				} else if (r < 0) {
					std::cerr << DBG_PREFIX"revents_handler announces exit with error" << std::endl;
					loop_exit = -1;
					break;
				}
			}

			// set pthis member of each event
			for (int i = 0; i < ret; ++i) {
				struct epoller_event *ev = (struct epoller_event*) revents[i].data.ptr;
				if (ev)
					ev->pthis = (struct epoller_event **) &revents[i].data.ptr;
				else {
					std::cerr << DBG_PREFIX"unexpected null pointer to epoll event" << std::endl;
					loop_exit = -1;
					break;
				}
			}
			if (loop_exit)
				break;

			// call handler of each event
			for (int i = 0; i < ret; ++i)
				if (revents[i].events) {
					struct epoller_event *ev = (struct epoller_event*) revents[i].data.ptr;
					if (ev) {
						r = ev->handler(this, &revents[i]);
						if (r)
							break;
					}
				}

			// clear pthis member of each event
			for (int i = 0; i < ret; ++i) {
				struct epoller_event *ev = (struct epoller_event*) revents[i].data.ptr;
				if (ev)
					ev->pthis = 0;
			}

			// exit if demanded
			if (r > 0) {
				loop_exit = 1;
				break;
			} else if (r < 0) {
				std::cerr << DBG_PREFIX"epoll event handler announces exit with error" << std::endl;
				loop_exit = -1;
				break;
			}

		}

		// glib stuff
		if (!del_gevents_from_epoller(gfds_n)) {
			std::cerr << DBG_PREFIX"deleting glib events from epoller failed" << std::endl;
			loop_exit = -1;
			break;
		}
		gevents_added = false;;
		if (!g_main_context_acquire(context)) {
			std::cerr << DBG_PREFIX"acquiring glib context failed" << std::endl;
			loop_exit = -1;
			break;
		}
		if (g_main_context_check(context, g_priority, gfds, gfds_n))
			g_main_context_dispatch(context);
		g_main_context_release(context);

	} // while (!loop_exit)

	// check to remove gevents
	if (gevents_added)
		del_gevents_from_epoller(gfds_n);

	return loop_exit > 0;
}

void gepoller::set_context(GMainContext *context)
{
	if (this->context == context)
		return;

	if (this->context)
		g_main_context_unref(this->context);

	if (!context)
		return;

	this->context = g_main_context_ref(context);
}

bool gepoller::add_gevents_to_epoller(int gfds_n)
{
	for (int i = 0; i < gfds_n; ++i) {
		gevents[i].gfd = &gfds[i];
		if (!gevents[i].add_to_epoller(this))
			return false;
	}

	return true;
}

bool gepoller::del_gevents_from_epoller(int gfds_n)
{
	for (int i = 0; i < gfds_n; ++i) {
		if (!gevents[i].del_from_epoller(this))
			return false;
		gevents[i].gfd = 0;
	}

	return true;
}

