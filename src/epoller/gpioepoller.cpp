#include "epoller/gpioepoller.h"
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>
#include <sstream>
#include <iostream>

#define DBG_PREFIX "gpioepoller: "
#define GPIO_DIR   "/sys/class/gpio"

bool gpioepoller::init(int fd)
{
	if (this->fd != -1)
		return true;

	if (!fdepoller::init(fd, 0, 0, false, false, false)) // fd, rxsize, txsize, rxen, txen, en
		return false;

	return true;
}

bool gpioepoller::open(const std::string &pathname, int flags)
{
	if (this->fd != -1)
		return true;

	int fd = ::open(pathname.c_str(), flags);
	if (fd == -1) {
		perror(DBG_PREFIX"opening file failed");
		return false;
	}

	if (!init(fd)) {
		::close(fd);
		return false;
	}

	return true;
}

bool gpioepoller::open(int gpio, int flags)
{
	std::stringstream path;
	path << GPIO_DIR << "/gpio" << gpio << "/value";
	return open(path.str(), flags);
}

bool gpioepoller::set_value(int value)
{
	if (lseek(fd, 0, SEEK_SET) == (off_t) -1) {
		perror(DBG_PREFIX"seeking failed");
		return false;
	}

	if (write(fd, value ? "1" : "0", 1) != 1) {
		perror(DBG_PREFIX"writing failed");
		return false;
	}

	return true;
}

bool gpioepoller::get_value(int *value)
{
	if (lseek(fd, 0, SEEK_SET) == (off_t) -1) {
		perror(DBG_PREFIX"seeking failed");
		return false;
	}

	if (read(fd, value, 1) != 1) {
		perror(DBG_PREFIX"reading failed");
		return false;
	}

	*value = *((char*)value) == '0' ? 0 : 1;
	return true;
}

bool gpioepoller::enable_irq()
{
	return enable(false, false, true); // rxen, txen, prien
}

bool gpioepoller::disable_irq()
{
	return disable();
}

int gpioepoller::irq()
{
	return _irq ? _irq(this) : 0;
}

int gpioepoller::epoll_pri()
{
	return irq();
}

int gpioepoller::epoll_err()
{
	return 0;
}

