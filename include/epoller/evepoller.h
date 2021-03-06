/// @file   epoller/evepoller.h
/// @author speedak
/// @brief  Event file descriptor wrapper.

#ifndef EVEPOLLER_H
#define EVEPOLLER_H

#include <epoller/epoller.h>
#include <sys/eventfd.h>
#include <stdint.h>

/// @brief Event epoller.
struct evepoller : epoller_event
{
	/// @brief Event receiver interface.
	struct receiver
	{
		/// @brief Destructor.
		virtual ~receiver() {}

		/// @brief Called when event is received.
		///        Default implementation returns -1.
		/// @param sender event sender
		/// @param cnt counter
		/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
		virtual int recv_handler(evepoller &sender, uint64_t cnt);
	};

	int                fd;      ///< event file descriptor
	struct epoller    *epoller; ///< parent epoller
	struct epoll_event event;   ///< epoll event
	struct receiver   *rcvr;    ///< event receiver

	/// @brief Called when event is received.
	/// @param sender event sender
	/// @param cnt counter
	/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
	int (*_recv_handler) (evepoller &sender, uint64_t cnt);

	/// @brief Constructor.
	/// @param epoller parent epoller
	evepoller(struct epoller *epoller) : fd(-1), epoller(epoller), event(), rcvr(0), _recv_handler(0) {}

	/// @brief Default constructor.
	evepoller() : evepoller(0) {}

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
	virtual bool send(uint64_t cnt = 1);

	/// @brief Called when event is received.
	///
	/// Default implementation calls receiver::recv_handler method of #rcvr if not null,
	/// otherwise calls #_recv_handler if not null,
	/// otherwise returns -1.
	///
	/// @param cnt counter
	/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
	virtual int recv_handler(uint64_t cnt);
};

#endif // EVEPOLLER_H

