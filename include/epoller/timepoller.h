/// @file   epoller/timepoller.h
/// @author speedak
/// @brief  Timer file descriptor wrapper.

#ifndef TIMEPOLLER_H
#define TIMEPOLLER_H

#include <epoller/epoller.h>
#include <stdint.h>
#include <sys/timerfd.h>

/// @brief Timer epoller.
struct timepoller : public epoller_event
{
	/// @brief Event receiver interface.
	class receiver
	{
	public:
		/// @brief Destructor.
		virtual ~receiver() {}

		/// @brief Called when timer event occurs.
		/// @param sender event sender
		/// @param exp number of expirations since last handler call
		/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
		virtual int timepoller_timerhandler(timepoller &sender, uint64_t exp) = 0;
	};

	int                fd;      ///< timer file descriptor
	struct epoller    *epoller; ///< parent epoller
	struct epoll_event event;   ///< epoll event
	struct receiver   *rcvr;    ///< event receiver

	/// @brief Handler for timer events.
	/// @see #timerhandler
	/// @param timepoller timer epoller within that the event occurred
	int (*_timerhandler) (struct timepoller *timepoller, uint64_t exp);

	/// @brief Constructor.
	/// @param epoller parent epoller
	timepoller(struct epoller *epoller) : fd(-1), epoller(epoller), event(), rcvr(0), _timerhandler(0) {}

	/// @brief Default constructor.
	timepoller() : timepoller(0) {}

	/// @brief Destructor.
	virtual ~timepoller() {cleanup();}

	/// @brief Initializes the timer epoller.
	/// @param clockid CLOCK_MONOTONIC or CLOCK_REALTIME, see documentation for timerfd_create syscall
	/// @return @c true if initialization was successful, otherwise @c false
	virtual bool init(int clockid = CLOCK_MONOTONIC);

	/// @brief Cleanups the timer epoller.
	virtual void cleanup();

	/// @brief Sets timer properties.
	/// @see timerfd_settime syscall documentation
	/// @return @c true if timer properties were written successfully, otherwise @c false
	bool settime(int flags, const struct itimerspec *new_value, struct itimerspec *old_value);

	/// @brief Gets timer properties.
	/// @see timerfd_gettime syscall documentation
	/// @return @c true if timer properties were read successfully, otherwise @c false
	bool gettime(struct itimerspec *curr_value);

	/// @brief Arms the oneshot timer.
	/// @see timerfd_settime syscall documentation
	/// @return @c true if timer was armed successfully, otherwise @c false
	bool arm_oneshot(const struct timespec *val, int flags = 0);

	/// @brief Arms the periodic timer.
	/// @see timerfd_settime syscall documentation
	/// @return @c true if timer was armed successfully, otherwise @c false
	bool arm_periodic(const struct timespec *val, const struct timespec *init_val = 0, int flags = 0);

	/// @brief Arms the timer.
	/// @see timerfd_settime syscall documentation
	/// @return @c true if timer was armed successfully, otherwise @c false
	bool arm(const struct itimerspec *val, int flags = 0);

	/// @brief Disarms the timer.
	/// @return @c true if timer was disarmed successfully, otherwise @c false
	bool disarm();

	/// @copydoc epoller_event::handler
	virtual int handler(struct epoller *epoller, struct epoll_event *revent);

	/// @brief Called when timer event occurs.
	///
	/// Default implementation calls receiver::timepoller_timerhandler method of #rcvr if not null,
	/// otherwise calls #_timerhandler if not null,
	/// otherwise returns 0.
	///
	/// @param exp number of expirations since last handler call
	/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
	virtual int timerhandler(uint64_t exp);
};

#endif // TIMEPOLLER_H

