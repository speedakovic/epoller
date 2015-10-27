/// @file   epoller/ttyepoller.h
/// @author speedak
/// @brief  TTY file descriptor wrapper.

#ifndef TTYEPOLLER_H
#define TTYEPOLLER_H

#include <epoller/fdepoller.h>
#include "termios.h"

/// @brief tty epoller based on file desciptor epoller.
struct ttyepoller : public fdepoller
{
	/// @brief Constructor.
	/// @param epoller parent epoller
	ttyepoller(struct epoller *epoller) : fdepoller(epoller) {}

	/// @brief Initializes the tty epoller.
	///
	/// @param fd
	/// @param rxsize
	/// @param txsize
	/// @param rxen
	/// @param txen
	/// @param en
	///
	/// @see fdepoller::init
	virtual bool init(int fd, size_t rxsize = 1024, size_t txsize = 1024, bool rxen = true, bool txen = false, bool en = true);

	/// @brief Opens tty and initializes the tty epoller.
	///
	/// @param pathname
	/// @param flags
	/// @param rxsize
	/// @param txsize
	/// @param rxen
	/// @param txen
	/// @param en
	///
	/// @see fdepoller::init
	virtual bool open(std::string pathname, int flags, size_t rxsize = 1024, size_t txsize = 1024, bool rxen = true, bool txen = false, bool en = true);

	/// @brief Configures the tty as raw serial line.
	///
	/// @param speed baud rate (B50, B75, B110, B134, B150, B200, B300, B600, B1200, B1800, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400)
	/// @param csize character size (CS5, CS6, CS7, CS8)
	/// @param parity parity (0 - none, 1 - even, 2 - odd)
	/// @param stopb number of stopbits (true - two, false - one)
	/// @param rtscts RTS/CTS flow control
	/// @param xonoff XON/XOFF flow control
	/// @param vmin minimum number of characters for read function
	/// @param vtime timeout in deciseconds for read function
	///
	/// @return @c true if configuration was successful, otherwise @c false
	bool raw_serial(speed_t speed, int csize, int parity, bool stopb, bool rtscts, bool xonoff, cc_t vmin = 0, cc_t vtime = 0);

};

#endif // TTYEPOLLER_H

