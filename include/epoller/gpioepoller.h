/// @file   epoller/gpioepoller.h
/// @author speedak
/// @brief  sysfs gpio's value file wrapper.

#ifndef GPIOEPOLLER_H
#define GPIOEPOLLER_H

#include <epoller/fdepoller.h>

/// @brief sysfs gpio's value file wrapper.
struct gpioepoller : fdepoller
{
	/// @brief gpio direction
	enum DIRECTION {
		DIRECTION_IN,   ///< input
		DIRECTION_OUT,  ///< output
		DIRECTION_LOW,  ///< output low
		DIRECTION_HIGH, ///< output high
	};

	/// @brief gpio edge
	enum EDGE {
		EDGE_NONE,    ///< none
		EDGE_RISING,  ///< rising
		EDGE_FALLING, ///< falling
		EDGE_BOTH     ///< both
	};

	/// @brief Event receiver interface.
	struct receiver : virtual fdepoller::receiver
	{
		/// @brief Destructor.
		virtual ~receiver() {}

		/// @brief Called if gpio irq occurs.
		///        Default implementation returns -1.
		/// @param sender event sender
		/// @param value gpio value, 0 or 1 if reading of gpio value was successful, otherwise -1
		/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
		virtual int irq(gpioepoller &sender, int value);
	};

	/// @brief Called if gpio irq occurs.
	/// @param sender event sender
	/// @param value gpio value, 0 or 1 if reading of gpio value was successful, otherwise -1
	/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
	int (*_irq) (gpioepoller &sender, int value);

	/// @brief Constructor.
	/// @param epoller parent epoller
	gpioepoller(struct epoller *epoller) : fdepoller(epoller), _irq(0) {}

	/// @brief Default constructor.
	gpioepoller() : gpioepoller(0) {}

	/// @brief Initializes the gpio epoller.
	/// @param fd file descriptor of the gpio's value file
	/// @return @c true if initialization was successful, otherwise @c false
	virtual bool init(int fd);

	/// @brief Opens file and initializes the gpio epoller.
	/// @see #init
	/// @param pathname name of the gpio's value file
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
	virtual bool set(int value);

	/// @brief Gets gpio value.
	/// @param value gpio value, 0 or 1
	/// @return @c true if getting was successful, otherwise @c false
	virtual bool get(int *value);

	/// @brief Enables gpio interrupts.
	/// @return @c true if enabling was successful, otherwise @c false
	virtual bool enable_irq();

	/// @brief Disables gpio interrupts.
	/// @return @c true if disabling was successful, otherwise @c false
	virtual bool disable_irq();

	/// @brief Called if gpio irq occurs.
	///
	/// Default implementation calls receiver::irq method of #rcvr if not null,
	/// otherwise calls #_irq if not null,
	/// otherwise returns 0.
	///
	/// @param value gpio value, 0 or 1 if reading of gpio value was successful, otherwise -1
	/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
	virtual int irq(int value);

	/// @brief EPOLLPRI event handler.
	///        This event means an interrupt on sysfs gpio.
	/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
	virtual int epoll_pri();

	/// @brief EPOLLERR event handler.
	/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
	virtual int epoll_err();

	/// @brief Sets gpio direction.
	/// @param pathname name of the gpio's direction file
	/// @param direction direction
	/// @return @c true if setting was successful, otherwise @c false
	static bool set_direction(const std::string &pathname, gpioepoller::DIRECTION direction);

	/// @brief Sets gpio direction.
	/// @param gpio gpio number
	/// @param direction direction
	/// @return @c true if setting was successful, otherwise @c false
	static bool set_direction(int gpio, gpioepoller::DIRECTION direction);

	/// @brief Gets gpio direction.
	/// @param pathname name of the gpio's direction file
	/// @param direction direction
	/// @return @c true if getting was successful, otherwise @c false
	static bool get_direction(const std::string &pathname, gpioepoller::DIRECTION *direction);

	/// @brief Gets gpio direction.
	/// @param gpio gpio number
	/// @param direction direction
	/// @return @c true if getting was successful, otherwise @c false
	static bool get_direction(int gpio, gpioepoller::DIRECTION *direction);

	/// @brief Sets gpio edge.
	/// @param pathname name of the gpio's edge file
	/// @param edge edge
	/// @return @c true if setting was successful, otherwise @c false
	static bool set_edge(const std::string &pathname, gpioepoller::EDGE edge);

	/// @brief Sets gpio edge.
	/// @param gpio gpio number
	/// @param edge edge
	/// @return @c true if setting was successful, otherwise @c false
	static bool set_edge(int gpio, gpioepoller::EDGE edge);

	/// @brief Gets gpio edge.
	/// @param pathname name of the gpio's edge file
	/// @param edge edge
	/// @return @c true if getting was successful, otherwise @c false
	static bool get_edge(const std::string &pathname, gpioepoller::EDGE *edge);

	/// @brief Gets gpio edge.
	/// @param gpio gpio number
	/// @param edge edge
	/// @return @c true if getting was successful, otherwise @c false
	static bool get_edge(int gpio, gpioepoller::EDGE *edge);

	/// @brief Sets gpio value.
	/// @param pathname name of the gpio's value file
	/// @param value value, zero is interpreted as '0', nonzero as '1'
	/// @return @c true if setting was successful, otherwise @c false
	static bool set_value(const std::string &pathname, int value);

	/// @brief Sets gpio value.
	/// @param gpio gpio number
	/// @param value value, zero is interpreted as '0', nonzero as '1'
	/// @return @c true if setting was successful, otherwise @c false
	static bool set_value(int gpio, int value);

	/// @brief Gets gpio value.
	/// @param pathname name of the gpio's value file
	/// @param value value, 0 or 1
	/// @return @c true if getting was successful, otherwise @c false
	static bool get_value(const std::string &pathname, int *value);

	/// @brief Gets gpio value.
	/// @param gpio gpio number
	/// @param value value, 0 or 1
	/// @return @c true if getting was successful, otherwise @c false
	static bool get_value(int gpio, int *value);
};

#endif // GPIOEPOLLER_H

