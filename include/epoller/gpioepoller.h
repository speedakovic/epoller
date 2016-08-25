/// @file   epoller/gpioepoller.h
/// @author speedak
/// @brief  sysfs gpio's value file wrapper.

#ifndef GPIOEPOLLER_H
#define GPIOEPOLLER_H

#include <epoller/fdepoller.h>

/// @brief sysfs gpio's value file wrapper.
struct gpioepoller : public fdepoller
{
	/// @brief Gpio irq handler.
	/// @see #irq
	/// @param irqepoller gpio epoller within that the irq occured
	int (*_irq) (struct gpioepoller *gpioepoller);

	/// @brief Constructor.
	/// @param epoller parent epoller
	gpioepoller(struct epoller *epoller) : fdepoller(epoller) {}

	/// @brief Initializes the gpio epoller.
	/// @param fd file descriptor
	/// @return @c true if initialization was successful, otherwise @c false
	virtual bool init(int fd);

	/// @brief Opens file and initializes the gpio epoller.
	/// @see #init
	/// @param pathname name of the file to be open
	/// @param flags O_RDONLY, O_WRONLY, O_RDWR, ... (see doc for posix open)
	virtual bool open(const std::string &pathname, int flags);

	/// @brief Opens gpio's value file and initializes the gpio epoller.
	/// @see #init
	/// @param gpio gpio number
	/// @param flags O_RDONLY, O_WRONLY, O_RDWR, ... (see doc for posix open)
	virtual bool open(int gpio, int flags);

	/// @brief Sets gpio value.
	/// @param value gpio value, zero is interpreted as '0', nonzero as '1'
	/// @return @c true if setting was successful, otherwise @c false
	virtual bool set_value(int value);

	/// @brief Gets gpio value.
	/// @param value gpio value, 0 or 1
	/// @return @c true if getting was successful, otherwise @c false
	virtual bool get_value(int *value);

	/// @brief Enables gpio interrupts.
	/// @return @c true if enabling was successful, otherwise @c false
	virtual bool enable_irq();

	/// @brief Disables gpio interrupts.
	/// @return @c true if disabling was successful, otherwise @c false
	virtual bool disable_irq();

	/// @brief Called if gpio irq occurs.
	///        Default implementation calls #_irq if not null, otherwise returns 0.
	/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
	virtual int irq();

	/// @brief EPOLLPRI event handler.
	///        This event means an interrupt on sysfs gpio.
	/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
	virtual int epoll_pri();

	/// @brief EPOLLERR event handler.
	/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
	virtual int epoll_err();
};

#endif // GPIOEPOLLER_H

