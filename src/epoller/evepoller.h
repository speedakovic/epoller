/// @file   evepoller.h
/// @author speedak
/// @brief  Event file descriptor wrapper.

#ifndef EVEPOLLER_H
#define EVEPOLLER_H

#include <epoller/epoller.h>
#include <sys/eventfd.h>
#include <stdint.h>

/// @brief Event epoller.
struct evepoller : public epoller_event
{
	int                fd;      ///< event file descriptor
	struct epoller    *epoller; ///< parent epoller
	struct epoll_event event;   ///< epoll event

	/// @brief Handler for received event.
	/// @see #recv_handler
	/// @param evepoller event epoller within that the event was received
	int (*_recv_handler) (struct evepoller *evepoller, uint64_t cnt);

	/// @brief Constructor.
	/// @param epoller parent epoller
	evepoller(struct epoller *epoller) : fd(-1), epoller(epoller), event(), _recv_handler(0) {}

	/// @brief Destructor.
	virtual ~evepoller() {cleanup();}

	/// @brief Initializes the event epoller.
	/// @param cnt initial counter passed to eventfd syscall
	/// @param flags flags passed to eventfd syscall
	/// @return @c true if initialization was successful, otherwise @c false
	virtual bool init(unsigned int cnt = 0, int flags = 0);

	/// @brief Cleanups the event epoller.
	virtual void cleanup();

	/// @copydoc epoller_event::handler
	virtual int handler(struct epoller *epoller, struct epoll_event *revent);

	/// @brief Sends event.
	/// @param cnt counter
	/// @return @c true if sending was successful, otherwise @c false
	virtual bool send(uint64_t cnt);

	/// @brief Called when event is received.
	///
	/// Default implementation calls #_recv_handler if not null, otherwise returns 0.
	///
	/// @param cnt counter
	/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
	virtual int recv_handler(uint64_t cnt);
};

#endif // EVEPOLLER_H

