#include <epoller/fdepoller.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <cstdio>
#include <cstring>

#define DBG_PREFIX "fdepoller: "

bool fdepoller::init(int fd, size_t rxsize, size_t txsize, bool rxen, bool txen, bool en)
{
	// check file descriptor
	if (this->fd != -1)
		return true; // already initialized

	// check given file descriptor
	if (fd < 0) {
		std::cerr << DBG_PREFIX"gots wrong file descriptor" << std::endl;
		goto unwind;
	}

	this->fd = fd;
	epoll_in_cnt  = 0;
	epoll_out_cnt = 0;
	epoll_hup_cnt = 0;
	epoll_err_cnt = 0;

	// add file descriptor to parent epoller
	if (en)
		if (!enable(rxen, txen))
			goto unwind;

	// initialize rx buffer
	if (rxsize > 0) {
		if (!linbuff_alloc(&rxbuff, rxsize)) {
			std::cerr << DBG_PREFIX"rx buffer allocation failed" << std::endl;
			goto unwind;
		}
	} else
		memset(&rxbuff, 0, sizeof rxbuff);

	// initialize tx buffer
	if (txsize > 0) {
		if (!linbuff_alloc(&txbuff, txsize)) {
			std::cerr << DBG_PREFIX"tx buffer allocation failed" << std::endl;
			goto unwind_free_rxbuff;
		}
	} else
		memset(&txbuff, 0, sizeof txbuff);

	return true;

//unwind_free_txbuff:
//	if (txbuff.buff)
//		linbuff_free(&txbuff);
//

unwind_free_rxbuff:
	if (rxbuff.buff)
		linbuff_free(&rxbuff);

unwind:
	this->fd = -1;
	return false;
}

void fdepoller::cleanup()
{
	// check file descriptor
	if (fd == -1)
		return; // already cleaned-up

	// free tx buffer
	if (txbuff.buff)
		linbuff_free(&txbuff);

	// free rx buffer
	if (rxbuff.buff)
		linbuff_free(&rxbuff);

	// remove file descriptor from parent epoller
	disable();

	// invalidate file descriptor
	fd = -1;
}

bool fdepoller::open(const std::string &pathname, int flags, size_t rxsize, size_t txsize, bool rxen, bool txen, bool en)
{
	if (this->fd != -1)
		return true;

	int fd = ::open(pathname.c_str(), flags);
	if (fd == -1) {
		perror(DBG_PREFIX"opening file failed");
		return false;
	}

	if (!init(fd, rxsize, txsize, rxen, txen, en)) {
		::close(fd);
		return false;
	}

	return true;
}

void fdepoller::close()
{
	if (this->fd == -1)
		return;

	int fd = this->fd;
	cleanup();
	::close(fd);
}

bool fdepoller::enable(bool rxen, bool txen)
{
	if (enabled)
		return true;

	memset(&event, 0, sizeof event);
	event.data.ptr = this;
	if (rxen)
		event.events |= EPOLLIN;
	if (txen)
		event.events |= EPOLLOUT;
	int ret = epoll_ctl(epoller->fd, EPOLL_CTL_ADD, fd, &event);
	if (ret == -1) {
		perror(DBG_PREFIX"adding file descriptor to parent epoller failed");
		return false;
	}

	enabled = true;
	return true;
}

bool fdepoller::disable()
{
	if (!enabled)
		return true;

	int ret = epoll_ctl(epoller->fd, EPOLL_CTL_DEL, fd, NULL);
	if (ret == -1) {
		perror(DBG_PREFIX"removing file descriptor from parent epoller failed");
		return false;
	}

	enabled = false;
	return true;
}

bool fdepoller::disable_rx()
{
	if (!(event.events & EPOLLIN))
		return true;

	event.events &= ~EPOLLIN;
	if (epoll_ctl(epoller->fd, EPOLL_CTL_MOD, fd, &event) == -1) {
		perror(DBG_PREFIX"modifying within epoller (clear EPOLLIN) failed");
		return false;
	} else
		return true;
}

bool fdepoller::enable_rx()
{
	if (event.events & EPOLLIN)
		return true;

	event.events |= EPOLLIN;
	if (epoll_ctl(epoller->fd, EPOLL_CTL_MOD, fd, &event) == -1) {
		perror(DBG_PREFIX"modifying within epoller (set EPOLLIN) failed");
		return false;
	} else
		return true;
}

bool fdepoller::disable_tx()
{
	if (!(event.events & EPOLLOUT))
		return true;

	event.events &= ~EPOLLOUT;
	if (epoll_ctl(epoller->fd, EPOLL_CTL_MOD, fd, &event) == -1) {
		perror(DBG_PREFIX"modifying within epoller (clear EPOLLOUT) failed");
		return false;
	} else
		return true;
}

bool fdepoller::enable_tx()
{
	if (event.events & EPOLLOUT)
		return true;

	event.events |= EPOLLOUT;
	if (epoll_ctl(epoller->fd, EPOLL_CTL_MOD, fd, &event) == -1) {
		perror(DBG_PREFIX"modifying within epoller (set EPOLLOUT) failed");
		return false;
	} else
		return true;
}

bool fdepoller::set_flags(int flags)
{
	int ret = fcntl(fd, F_GETFL);
	if (ret == -1) {
		perror(DBG_PREFIX"getting file descriptor flags failed");
		return false;
	}

	ret = fcntl(fd, F_SETFL, ret | flags);
	if (ret == -1) {
		perror(DBG_PREFIX"setting file descriptor flags failed");
		return false;
	}

	return true;
}

bool fdepoller::clear_flags(int flags)
{
	int ret = fcntl(fd, F_GETFL);
	if (ret == -1) {
		perror(DBG_PREFIX"getting file descriptor flags failed");
		return false;
	}

	ret = fcntl(fd, F_SETFL, ret & ~flags);
	if (ret == -1) {
		perror(DBG_PREFIX"setting file descriptor flags failed");
		return false;
	}

	return true;
}

bool fdepoller::get_flags(int *flags)
{
	int ret = fcntl(fd, F_GETFL);
	if (ret == -1) {
		perror(DBG_PREFIX"getting file descriptor flags failed");
		return false;
	}

	*flags = ret;
	return true;
}

int fdepoller::rx(int len)
{
	if (_rx)
		return _rx(this, len);
	else {
		std::cerr << DBG_PREFIX"rx event, len = " << len << std::endl;
		if (len == 0)
			return 1;
		else if (len < 0)
			return -1;
		else {
			linbuff_clear(&rxbuff);
			return 0;
		}
	}
}

int fdepoller::tx(int len)
{
	if (_tx)
		return _tx(this, len);
	else {
		std::cerr << DBG_PREFIX"tx event, len = " << len << std::endl;
		if (len == 0)
			return 1;
		else if (len < 0)
			return -1;
		else {
			linbuff_compact(&txbuff);
			return 0;
		}
	}
}

int fdepoller::hup()
{
	if (_hup)
		return _hup(this);
	else {
		std::cerr << DBG_PREFIX"hang-out event" << std::endl;
		return -1;
	}
}

int fdepoller::err()
{
	if (_err)
		return _err(this);
	else {
		std::cerr << DBG_PREFIX"error event" << std::endl;
		return -1;
	}
}

int fdepoller::un(int events)
{
	if (_un)
		return _un(this, events);
	else {
		std::cerr << DBG_PREFIX"unknown event" << std::endl;
		return -1;
	}
}

int fdepoller::enter(struct epoll_event *revent)
{
	return _enter ? _enter(this, revent) : 0;
}

int fdepoller::exit(struct epoll_event *revent)
{
	return _exit ? _exit(this, revent) : 0;
}

int fdepoller::epoll_in()
{
	int ret = read(fd, LINBUFF_WR_PTR(&rxbuff), linbuff_towr(&rxbuff));

	if (ret < -1) {
		perror(DBG_PREFIX"reading from file descriptor failed (unexpected retvalue)");
		return rx(-1);

	} else if (ret == -1) {
		perror(DBG_PREFIX"reading from file descriptor failed");
		return rx(-1);

	} else if (ret == 0) {
		return rx(0);

	} else {
		linbuff_forward(&rxbuff, ret);
		return rx(ret);
	}
}

int fdepoller::epoll_out()
{
	int ret = write(fd, LINBUFF_RD_PTR(&txbuff), linbuff_tord(&txbuff));

	if (ret < -1) {
		perror(DBG_PREFIX"writing to file descriptor failed (unexpected retvalue)");
		return tx(-1);

	} else if (ret == -1) {
		perror(DBG_PREFIX"writing to file descriptor failed");
		return tx(-1);

	} else if (ret == 0) {
		return tx(0);

	} else {
		linbuff_skip(&txbuff, ret);
		return tx(ret);
	}
}

int fdepoller::epoll_hup()
{
	return hup();
}

int fdepoller::epoll_err()
{
	return err();
}

int fdepoller::epoll_unknown(int events)
{
	return un(events);
}

int fdepoller::handler(struct epoller *epoller, struct epoll_event *revent)
{
	int ret;
	struct epoller_event **pthis = epoller_event::pthis;

	ret = enter(revent);
	if (ret || !*pthis)
		return ret;

	if (revent->events & EPOLLIN) {
		revent->events &= ~EPOLLIN;
		epoll_in_cnt++;
		ret = epoll_in();
		if (ret || !*pthis)
			return ret;
	}

	if (revent->events & EPOLLOUT) {
		revent->events &= ~EPOLLOUT;
		epoll_out_cnt++;
		ret = epoll_out();
		if (ret || !*pthis)
			return ret;
	}

	if (revent->events & EPOLLHUP) {
		revent->events &= ~EPOLLHUP;
		epoll_hup_cnt++;
		ret = epoll_hup();
		if (ret || !*pthis)
			return ret;
	}

	if (revent->events & EPOLLERR) {
		revent->events &= ~EPOLLERR;
		epoll_err_cnt++;
		ret = epoll_err();
		if (ret || !*pthis)
			return ret;
	}

	if (revent->events) {
		ret = epoll_unknown(revent->events);
		if (ret || !*pthis)
			return ret;
	}

	if (linbuff_towr(&rxbuff)) {
		if (enabled && rx_auto_enable && !enable_rx())
			return -1;
	} else {
		if (enabled && rx_auto_disable && !disable_rx())
			return -1;
	}

	if (linbuff_tord(&txbuff)) {
		if (enabled && tx_auto_enable && !enable_tx())
			return -1;
	} else {
		if (enabled && tx_auto_disable && !disable_tx())
			return -1;
	}

	ret = exit(revent);
	if (ret || !*pthis)
		return ret;

	return 0;
}

