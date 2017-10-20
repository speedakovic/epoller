#include <epoller/sigepoller.h>
#include <unistd.h>
#include <iostream>
#include <cstdio>
#include <cstring>

#define DBG_PREFIX "sigepoller: "

bool sigepoller::init(const sigset_t *sigset)
{
	int ret;

	// check file descriptor
	if (fd != -1) {
		std::cerr << DBG_PREFIX"already initialized" << std::endl;
		goto unwind;
	}

	// create signal file descriptor
	fd = signalfd(-1, sigset, 0);
	if (fd == -1) {
		perror(DBG_PREFIX"file descriptor creation failed");
		goto unwind;
	}

	// add signal file descriptor to epoller
	memset(&event, 0, sizeof event);
	event.data.ptr = this;
	event.events = EPOLLIN;
	ret = epoll_ctl(epoller->fd, EPOLL_CTL_ADD, fd, &event);
	if (ret == -1) {
		perror(DBG_PREFIX"adding file descriptor to epoller failed");
		goto unwind_fd;
	}

	return true;

unwind_fd:
	close(fd);
	fd = -1;

unwind:
	return false;
}

void sigepoller::cleanup()
{
	// check file descriptor
	if (fd == -1)
		return; // already cleaned-up

	// remove signal file descriptor from epoller
	if (epoller->fd != -1 && epoll_ctl(epoller->fd, EPOLL_CTL_DEL, fd, NULL) == -1)
		perror(DBG_PREFIX"removing file descriptor from epoller failed");

	// close and invalidate signal file descriptor
	close(fd);
	fd = -1;
}

int sigepoller::handler(struct epoller *epoller, struct epoll_event *revent)
{
	int ret;
	struct signalfd_siginfo siginfo;

	if (revent->events & EPOLLIN) {
		revent->events &= ~EPOLLIN;

		ret = read(fd, &siginfo, sizeof siginfo);
		if (ret == -1) {
			perror(DBG_PREFIX"reading from file descriptor failed");
			return -1;

		} else if (ret == 0) {
			perror(DBG_PREFIX"no data read from file descriptor");
			return -1;

		} else {
			if (ret != sizeof siginfo) {
				perror(DBG_PREFIX"mismatched data read from file descriptor");
				return -1;
			}

			return sighandler(&siginfo);
		}
	}

	if (revent->events & EPOLLHUP) {
		revent->events &= ~EPOLLHUP;
		std::cerr << DBG_PREFIX"unexpected EPOLLHUP on file descriptor" << std::endl;
		return -1;
	}

	if (revent->events & EPOLLERR) {
		revent->events &= ~EPOLLERR;
		std::cerr << DBG_PREFIX"unexpected EPOLLERR on file descriptor" << std::endl;
		return -1;
	}

	if (revent->events) {
		std::cerr << DBG_PREFIX"unexpected unknown event on file descriptor, events = " << revent->events << std::endl;
		return -1;
	}

	return 0;
}

int sigepoller::sighandler(struct signalfd_siginfo *siginfo)
{
	return rcvr ? rcvr->sighandler(*this, siginfo) : (_sighandler ? _sighandler(this, siginfo) : 0);
}

