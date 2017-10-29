#include <epoller/inotepoller.h>
#include <unistd.h>
#include <climits>
#include <iostream>
#include <cstdio>
#include <cstring>

#define DBG_PREFIX "inotepoller: "

int inotepoller::receiver::inothandler(inotepoller &sender, struct inotify_event *event)
{
	std::cerr << DBG_PREFIX"unhandled event: inothandler" << std::endl;
	return -1;
}

bool inotepoller::init()
{
	int ret;

	// check file descriptor
	if (fd != -1) {
		std::cerr << DBG_PREFIX"already initialized" << std::endl;
		goto unwind;
	}

	// create inotify file descriptor
	fd = inotify_init();
	if (fd == -1) {
		perror(DBG_PREFIX"file descriptor creation failed");
		goto unwind;
	}

	// add inotify file descriptor to epoller
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

void inotepoller::cleanup()
{
	// check file descriptor
	if (fd == -1)
		return; // already cleaned-up

	// remove inotify file descriptor from epoller
	if (epoller->fd != -1 && epoll_ctl(epoller->fd, EPOLL_CTL_DEL, fd, NULL) == -1)
		perror(DBG_PREFIX"removing file descriptor from epoller failed");

	// close and invalidate inotify file descriptor
	close(fd);
	fd = -1;
}

int inotepoller::add_watch(const std::string &pathname, uint32_t mask)
{
	int wd = inotify_add_watch(fd, pathname.c_str(), mask);
	if (wd == -1)
		perror(DBG_PREFIX"adding watch failed");

	return wd;
}

bool inotepoller::rm_watch(int wd)
{
	int ret = inotify_rm_watch(fd, wd);
	if (ret == -1) {
		perror(DBG_PREFIX"removing watch failed");
		return false;
	}

	return true;
}

int inotepoller::handler(struct epoller *epoller, struct epoll_event *revent)
{
	int ret;
	uint8_t buff[sizeof(struct inotify_event) + NAME_MAX + 1];
	struct inotify_event *event = (struct inotify_event *)buff;

	if (revent->events & EPOLLIN) {
		revent->events &= ~EPOLLIN;

		ret = read(fd, event, sizeof buff);
		if (ret == -1) {
			perror(DBG_PREFIX"reading from file descriptor failed");
			return -1;

		} else if (ret == 0) {
			perror(DBG_PREFIX"no data read from file descriptor");
			return -1;

		} else {
			if (ret < (int)sizeof(struct inotify_event)) {
				perror(DBG_PREFIX"mismatched data read from file descriptor");
				return -1;
			}

			return inothandler(event);
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

int inotepoller::inothandler(struct inotify_event *event)
{
	if (rcvr)
		return rcvr->inothandler(*this, event);
	else if (_inothandler)
		return _inothandler(*this, event);
	else {
		std::cerr << DBG_PREFIX"unhandled event: inothandler" << std::endl;
		return -1;
	}
}

