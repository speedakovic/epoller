#include <epoller/epoller.h>
#include <unistd.h>
#include <cstdio>
#include <iostream>

#define DBG_PREFIX "epoller: "

bool epoller::init()
{
	// check epoll file descriptor
	if (fd != -1) {
		std::cerr << DBG_PREFIX"already initialized" << std::endl;
		return false;
	}

	// create epoll file descriptor
	fd = epoll_create(1);
	if (fd == -1) {
		perror(DBG_PREFIX"epoll file descriptor creation failed");
		return false;
	}

	return true;
}

void epoller::cleanup()
{
	// check epoll file descriptor
	if (fd == -1) {
		std::cerr << DBG_PREFIX"not initialized yet" << std::endl;
		return;
	}

	// close epoll file descriptor
	close(fd);
	fd = -1;
}

bool epoller::loop()
{
	int r, ret;

	loop_exit = 0;
	while (!loop_exit) {

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

		// call epoll wait
		ret = epoll_wait(fd, revents, revents_size, timeout);

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
	} // while (!loop_exit)

	return loop_exit > 0;
}

void epoller::exit(int how)
{
	loop_exit = how;
}

