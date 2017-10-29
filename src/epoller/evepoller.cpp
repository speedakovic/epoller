#include <epoller/evepoller.h>
#include <unistd.h>
#include <iostream>
#include <cstdio>
#include <cstring>

#define DBG_PREFIX "evepoller: "

int evepoller::receiver::recv_handler(evepoller &sender, uint64_t cnt)
{
	std::cerr << DBG_PREFIX"unhandled event: recv_handler" << std::endl;
	return -1;
}

bool evepoller::init(unsigned int cnt, int flags)
{
	int ret;

	// check file descriptor
	if (fd != -1) {
		std::cerr << DBG_PREFIX"already initialized" << std::endl;
		goto unwind;
	}

	// create event file descriptor
	fd = eventfd(cnt, flags);
	if (fd == -1) {
		perror(DBG_PREFIX"file descriptor creation failed");
		goto unwind;
	}

	// add event file descriptor to epoller
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

void evepoller::cleanup()
{
	// check file descriptor
	if (fd == -1)
		return; // already cleaned-up

	// remove event file descriptor from epoller
	if (epoller->fd != -1 && epoll_ctl(epoller->fd, EPOLL_CTL_DEL, fd, NULL) == -1)
		perror(DBG_PREFIX"removing file descriptor from epoller failed");

	// close and invalidate event file descriptor
	close(fd);
	fd = -1;
}

bool evepoller::send(uint64_t cnt)
{
	if (write(fd, &cnt, sizeof cnt) != sizeof cnt) {
		perror(DBG_PREFIX"writing to file descriptor failed");
		return false;
	}

	return true;
}

int evepoller::handler(struct epoller *epoller, struct epoll_event *revent)
{
	int ret;
	uint64_t cnt;

	if (revent->events & EPOLLIN) {
		revent->events &= ~EPOLLIN;

		ret = read(fd, &cnt, sizeof cnt);
		if (ret == -1) {
			perror(DBG_PREFIX"reading from file descriptor failed");
			return -1;

		} else if (ret == 0) {
			perror(DBG_PREFIX"no data read from file descriptor");
			return -1;

		} else {
			if (ret != sizeof cnt) {
				perror(DBG_PREFIX"mismatched data read from file descriptor");
				return -1;
			}

			return recv_handler(cnt);
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

int evepoller::recv_handler(uint64_t cnt)
{
	if (rcvr)
		return rcvr->recv_handler(*this, cnt);
	else if (_recv_handler)
		return _recv_handler(*this, cnt);
	else {
		std::cerr << DBG_PREFIX"unhandled event: recv_handler" << std::endl;
		return -1;
	}
}

