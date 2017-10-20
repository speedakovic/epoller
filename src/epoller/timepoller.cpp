#include <epoller/timepoller.h>
#include <unistd.h>
#include <iostream>
#include <cstdio>
#include <cstring>

#define DBG_PREFIX "timepoller: "

bool timepoller::init(int clockid)
{
	int ret;

	// check file descriptor
	if (fd != -1) {
		std::cerr << DBG_PREFIX"already initialized" << std::endl;
		goto unwind;
	}

	// create timer file descriptor
	fd = timerfd_create(clockid, 0);
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

void timepoller::cleanup()
{
	// check file descriptor
	if (fd == -1)
		return; // already cleaned-up

	// disarm timer
	disarm();

	// remove timer file descriptor from epoller
	if (epoller->fd != -1 && epoll_ctl(epoller->fd, EPOLL_CTL_DEL, fd, NULL) == -1)
		perror(DBG_PREFIX"removing file descriptor from epoller failed");

	// close and invalidate timer file descriptor
	close(fd);
	fd = -1;
}

bool timepoller::settime(int flags, const struct itimerspec *new_value, struct itimerspec *old_value)
{
	int ret = timerfd_settime(fd, flags, new_value, old_value);
	if (ret == -1) {
		perror(DBG_PREFIX"setting file descriptor properties failed (settime)");
		return false;
	} else
		return true;
}

bool timepoller::gettime(struct itimerspec *curr_value)
{
	int ret = timerfd_gettime(fd, curr_value);
	if (ret == -1) {
		perror(DBG_PREFIX"getting file descriptor properties failed (gettime)");
		return false;
	} else
		return true;
}

bool timepoller::arm_oneshot(const struct timespec *val, int flags)
{
	struct itimerspec spec = {};
	spec.it_value = *val;

	int ret = timerfd_settime(fd, flags, &spec, NULL);
	if (ret == -1) {
		perror(DBG_PREFIX"setting file descriptor properties failed (arm_oneshot)");
		return false;
	} else
		return true;
}

bool timepoller::arm_oneshot_msec(uint64_t msec, int flags)
{
	struct timespec ts = {};
	ts.tv_sec  =  msec / 1000ULL;
	ts.tv_nsec = (msec % 1000ULL) * 1000000ULL;

	return arm_oneshot(&ts, flags);
}

bool timepoller::arm_oneshot_usec(uint64_t usec, int flags)
{
	struct timespec ts = {};
	ts.tv_sec  =  usec / 1000000ULL;
	ts.tv_nsec = (usec % 1000000ULL) * 1000ULL;

	return arm_oneshot(&ts, flags);
}

bool timepoller::arm_periodic(const struct timespec *val, const struct timespec *init_val, int flags)
{
	struct itimerspec spec = {};
	spec.it_interval = *val;
	spec.it_value = init_val ? *init_val : *val;

	int ret = timerfd_settime(fd, flags, &spec, NULL);
	if (ret == -1) {
		perror(DBG_PREFIX"setting file descriptor properties failed (arm_periodic)");
		return false;
	} else
		return true;
}

bool timepoller::arm_periodic_msec(uint64_t msec, uint64_t init_msec, int flags)
{
	struct timespec ts = {};
	ts.tv_sec  =  msec / 1000ULL;
	ts.tv_nsec = (msec % 1000ULL) * 1000000ULL;

	struct timespec init_ts = {};
	init_ts.tv_sec  =  init_msec / 1000ULL;
	init_ts.tv_nsec = (init_msec % 1000ULL) * 1000000ULL;

	return arm_periodic(&ts, &init_ts, flags);
}

bool timepoller::arm_periodic_usec(uint64_t usec, uint64_t init_usec, int flags)
{
	struct timespec ts = {};
	ts.tv_sec  =  usec / 1000000ULL;
	ts.tv_nsec = (usec % 1000000ULL) * 1000ULL;

	struct timespec init_ts = {};
	init_ts.tv_sec  =  init_usec / 1000000ULL;
	init_ts.tv_nsec = (init_usec % 1000000ULL) * 1000ULL;

	return arm_periodic(&ts, &init_ts, flags);
}

bool timepoller::arm(const struct itimerspec *val, int flags)
{
	int ret = timerfd_settime(fd, flags, val, NULL);
	if (ret == -1) {
		perror(DBG_PREFIX"setting file descriptor properties failed (arm)");
		return false;
	} else
		return true;
}

bool timepoller::disarm()
{
	struct itimerspec spec = {};

	int ret = timerfd_settime(fd, 0, &spec, NULL);
	if (ret == -1) {
		perror(DBG_PREFIX"setting file descriptor properties failed (disarm)");
		return false;
	} else
		return true;
}

int timepoller::handler(struct epoller *epoller, struct epoll_event *revent)
{
	int ret;
	uint64_t exp;

	if (revent->events & EPOLLIN) {
		revent->events &= ~EPOLLIN;

		ret = read(fd, &exp, sizeof exp);
		if (ret == -1) {
			perror(DBG_PREFIX"reading from file descriptor failed");
			return -1;

		} else if (ret == 0) {
			perror(DBG_PREFIX"no data read from file descriptor");
			return -1;

		} else {
			if (ret != sizeof exp) {
				perror(DBG_PREFIX"mismatched data read from file descriptor");
				return -1;
			}

			return timerhandler(exp);
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

int timepoller::timerhandler(uint64_t exp)
{
	return rcvr ? rcvr->timepoller_timerhandler(*this, exp) : (_timerhandler ? _timerhandler(this, exp) : 0);
}

