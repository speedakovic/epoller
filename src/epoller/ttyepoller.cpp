#include <epoller/ttyepoller.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>
#include <iostream>
#include <cstring>

#define DBG_PREFIX "ttyepoller: "

bool ttyepoller::init(int fd, size_t rxsize, size_t txsize, bool rxen, bool txen, bool en)
{
	if (this->fd != -1)
		return true;

	if (!isatty(fd)) {
		perror(DBG_PREFIX"checking tty failed");
		return false;
	}

	if(!fdepoller::init(fd, rxsize, txsize, rxen, txen, en))
		return false;

	return true;
}

bool ttyepoller::open(std::string pathname, int flags, size_t rxsize, size_t txsize, bool rxen, bool txen, bool en)
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

bool ttyepoller::raw_serial(speed_t speed, int csize, int parity, bool stopb, bool rtscts, bool xonoff, cc_t vmin, cc_t vtime)
{
	struct termios tio = {};

	// input flags
	tio.c_iflag  = 0;
	tio.c_iflag |= xonoff ? IXON | IXOFF : 0;

	// output flags
	tio.c_oflag = 0;

	// control flags
	tio.c_cflag  = CREAD | CLOCAL | csize;
	tio.c_cflag |= stopb ? CSTOPB : 0;
	tio.c_cflag |= rtscts ? CRTSCTS : 0;
	tio.c_cflag |= parity ? PARENB | (parity == 2 ? PARODD : 0) : 0;

	// local flags
	tio.c_lflag = 0;

	// control characters
	tio.c_cc[VMIN]   = vmin;
	tio.c_cc[VTIME]  = vtime;
	tio.c_cc[VSTART] = 021;
	tio.c_cc[VSTOP]  = 023;

	// fill speed values
	cfsetospeed(&tio, speed);
	cfsetispeed(&tio, speed);

	// set attributes to the tty
	if (tcsetattr(fd, TCSANOW, &tio)) {
		perror(DBG_PREFIX"setting tty attributes failed");
		return false;
	}

	// check the previously set attributes by reading-out and comparing them

	struct termios tio2 = {};
	if (tcgetattr(fd, &tio2)) {
		perror(DBG_PREFIX"getting tty attributes failed");
		return false;
	}

	if (memcmp(&tio, &tio2, sizeof(struct termios))) {
		std::cerr << DBG_PREFIX"tty attributes check failed" << std::endl;
		return false;
	}

	return true;
}

