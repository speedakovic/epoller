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

	for(;;) {

		// call pre-epoll handler
		if (pre_epoll_handler) {
			r = pre_epoll_handler(this);
			if (r > 0)
				return true;
			else if (r < 0) {
				std::cerr << DBG_PREFIX"pre_epoll_handler announces exit with error" << std::endl;
				return false;
			}
		}

		// call epoll wait
		ret = epoll_wait(fd, revents , revents_size, timeout);

		// call post-epoll handler
		if (post_epoll_handler) {
			r = post_epoll_handler(this);
			if (r > 0)
				return true;
			else if (r < 0) {
				std::cerr << DBG_PREFIX"post_epoll_handler announces exit with error" << std::endl;
				return false;
			}
		}

		if (ret == -1) {

			// error
			perror(DBG_PREFIX"epoll waiting failed");
			return false;

		} else if (ret == 0) {

			// call timeout handler
			if (timeout_handler) {
				r = timeout_handler(this);
				if (r > 0)
					return true;
				else if (r < 0) {
					std::cerr << DBG_PREFIX"timeout_handler announces exit with error" << std::endl;
					return false;
				}
			}

			continue;

		} else {

			// call revents handler
			if (revents_handler) {
				r = revents_handler(this, revents, ret);
				if (r > 0)
					return true;
				else if (r < 0) {
					std::cerr << DBG_PREFIX"revents_handler announces exit with error" << std::endl;
					return false;
				}
			}

			// set pthis member of each event
			for (int i = 0; i < ret; ++i) {
				struct epoller_event *ev = (struct epoller_event*) revents[i].data.ptr;
				if (ev)
					ev->pthis = (struct epoller_event **) &revents[i].data.ptr;
				else {
					std::cerr << DBG_PREFIX"unexpected null pointer to epoll event" << std::endl;
					return false;
				}
			}

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

			// return if demanded
			if (r > 0)
				return true;
			else if (r < 0) {
				std::cerr << DBG_PREFIX"epoll event handler announces exit with error" << std::endl;
				return false;
			}

		}
	}
}

