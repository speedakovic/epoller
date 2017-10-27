#include "epoller/gpioepoller.h"
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>
#include <cstring>
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

bool gpioepoller::set(int value)
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

bool gpioepoller::get(int *value)
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

int gpioepoller::irq(int value)
{
	return rcvr ? static_cast<receiver *>(rcvr)->irq(*this, value) : (_irq ? _irq(*this, value) : 0);
}

int gpioepoller::epoll_pri()
{
	int value;
	return get(&value) ? irq(value) : irq(-1);
}

int gpioepoller::epoll_err()
{
	return 0;
}

bool gpioepoller::set_direction(const std::string &pathname, gpioepoller::DIRECTION direction)
{
	int fd = ::open(pathname.c_str(), O_WRONLY);
	if (fd == -1) {
		perror(DBG_PREFIX"opening direction file failed");
		return false;
	}

	char *buff = 0;

	if (direction == DIRECTION_IN)
		buff = (char*) "in";
	else if (direction == DIRECTION_OUT)
		buff = (char*) "out";
	else if (direction == DIRECTION_LOW)
		buff = (char*) "low";
	else if (direction == DIRECTION_HIGH)
		buff = (char*) "high";
	else {
		std::cerr << DBG_PREFIX"invalid direction" << std::endl;
		::close(fd);
		return false;
	}

	if ((size_t) write(fd, buff, strlen(buff)) != strlen(buff)) {
		perror(DBG_PREFIX"writing direction file failed");
		::close(fd);
		return false;
	}

	::close(fd);
	return true;
}

bool gpioepoller::set_direction(int gpio, gpioepoller::DIRECTION direction)
{
	std::stringstream path;
	path << GPIO_DIR << "/gpio" << gpio << "/direction";
	return set_direction(path.str(), direction);
}

bool gpioepoller::get_direction(const std::string &pathname, gpioepoller::DIRECTION *direction)
{
	int fd = ::open(pathname.c_str(), O_RDONLY);
	if (fd == -1) {
		perror(DBG_PREFIX"opening direction file failed");
		return false;
	}

	char buff[3] = {};

	int ret = read(fd, buff, sizeof(buff));
	if (ret == -1) {
		perror(DBG_PREFIX"reading direction file failed");
		::close(fd);
		return false;
	}

	if (!memcmp(buff, "in", 2))
		*direction = DIRECTION_IN;
	else if (!memcmp(buff, "out", 3))
		*direction = DIRECTION_OUT;
	else {
		std::cerr << DBG_PREFIX"unknown value in direction file" << std::endl;
		::close(fd);
		return false;
	}

	::close(fd);
	return true;
}

bool gpioepoller::get_direction(int gpio, gpioepoller::DIRECTION *direction)
{
	std::stringstream path;
	path << GPIO_DIR << "/gpio" << gpio << "/direction";
	return get_direction(path.str(), direction);
}

bool gpioepoller::set_edge(const std::string &pathname, gpioepoller::EDGE edge)
{
	int fd = ::open(pathname.c_str(), O_WRONLY);
	if (fd == -1) {
		perror(DBG_PREFIX"opening edge file failed");
		return false;
	}

	char *buff = 0;

	if (edge == EDGE_NONE)
		buff = (char*) "none";
	else if (edge == EDGE_RISING)
		buff = (char*) "rising";
	else if (edge == EDGE_FALLING)
		buff = (char*) "falling";
	else if (edge == EDGE_BOTH)
		buff = (char*) "both";
	else {
		std::cerr << DBG_PREFIX"invalid edge" << std::endl;
		::close(fd);
		return false;
	}

	if ((size_t) write(fd, buff, strlen(buff)) != strlen(buff)) {
		perror(DBG_PREFIX"writing edge file failed");
		::close(fd);
		return false;
	}

	::close(fd);
	return true;
}

bool gpioepoller::set_edge(int gpio, gpioepoller::EDGE edge)
{
	std::stringstream path;
	path << GPIO_DIR << "/gpio" << gpio << "/edge";
	return set_edge(path.str(), edge);
}

bool gpioepoller::get_edge(const std::string &pathname, gpioepoller::EDGE *edge)
{
	int fd = ::open(pathname.c_str(), O_RDONLY);
	if (fd == -1) {
		perror(DBG_PREFIX"opening edge file failed");
		return false;
	}

	char buff[7] = {};

	int ret = read(fd, buff, sizeof(buff));
	if (ret == -1) {
		perror(DBG_PREFIX"reading edge file failed");
		::close(fd);
		return false;
	}

	if (!memcmp(buff, "none", 4))
		*edge = EDGE_NONE;
	else if (!memcmp(buff, "rising", 6))
		*edge = EDGE_RISING;
	else if (!memcmp(buff, "falling", 7))
		*edge = EDGE_FALLING;
	else if (!memcmp(buff, "both", 4))
		*edge = EDGE_BOTH;
	else {
		std::cerr << DBG_PREFIX"unknown value in edge file" << std::endl;
		::close(fd);
		return false;
	}

	::close(fd);
	return true;
}

bool gpioepoller::get_edge(int gpio, gpioepoller::EDGE *edge)
{
	std::stringstream path;
	path << GPIO_DIR << "/gpio" << gpio << "/edge";
	return get_edge(path.str(), edge);
}

bool gpioepoller::set_value(const std::string &pathname, int value)
{
	int fd = ::open(pathname.c_str(), O_WRONLY);
	if (fd == -1) {
		perror(DBG_PREFIX"opening value file failed");
		return false;
	}

	if (write(fd, value ? "1" : "0", 1) != 1) {
		perror(DBG_PREFIX"writing value file failed");
		::close(fd);
		return false;
	}

	::close(fd);
	return true;
}

bool gpioepoller::set_value(int gpio, int value)
{
	std::stringstream path;
	path << GPIO_DIR << "/gpio" << gpio << "/value";
	return set_value(path.str(), value);
}

bool gpioepoller::get_value(const std::string &pathname, int *value)
{
	int fd = ::open(pathname.c_str(), O_RDONLY);
	if (fd == -1) {
		perror(DBG_PREFIX"opening value file failed");
		return false;
	}

	if (read(fd, value, 1) != 1) {
		perror(DBG_PREFIX"reading value file failed");
		::close(fd);
		return false;
	}

	*value = *((char*)value) == '0' ? 0 : 1;

	::close(fd);
	return true;
}

bool gpioepoller::get_value(int gpio, int *value)
{
	std::stringstream path;
	path << GPIO_DIR << "/gpio" << gpio << "/value";
	return get_value(path.str(), value);
}

