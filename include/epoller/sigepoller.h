/// @file   epoller/sigepoller.h
/// @author speedak
/// @brief  Signal file descriptor wrapper.

#ifndef SIGEPOLLER_H
#define SIGEPOLLER_H

#include <epoller/epoller.h>
#include <sys/signalfd.h>

/// @brief Signal epoller.
struct sigepoller : epoller_event
{
	/// @brief Event receiver interface.
	struct receiver
	{
		/// @brief Destructor.
		virtual ~receiver() {}

		/// @brief Called when signal is received.
		///        Default implementation returns -1.
		/// @param sender event sender
		/// @param siginfo structure with information about the received signal
		/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
		virtual int sighandler(sigepoller &sender, struct signalfd_siginfo *siginfo);
	};

	int                fd;      ///< signal file descriptor
	struct epoller    *epoller; ///< parent epoller
	struct epoll_event event;   ///< epoll event
	struct receiver   *rcvr;    ///< event receiver

	/// @brief Called when signal is received.
	/// @param sender event sender
	/// @param siginfo structure with information about the received signal
	/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
	int (*_sighandler) (sigepoller &sender, struct signalfd_siginfo *siginfo);

	/// @brief Constructor.
	/// @param epoller parent epoller
	sigepoller(struct epoller *epoller) : fd(-1), epoller(epoller), event(), rcvr(0), _sighandler(0) {}

	/// @brief Default constructor.
	sigepoller() : sigepoller(0) {}

	/// @brief Destructor.
	virtual ~sigepoller() {cleanup();}

	/// @brief Initializes the signal epoller.
	/// @param sigset set of signals accepted by the signal epoller
	/// @return @c true if initialization was successful, otherwise @c false
	virtual bool init(const sigset_t *sigset);

	/// @brief Cleanups the signal epoller.
	virtual void cleanup();

	/// @copydoc epoller_event::handler
	virtual int handler(struct epoller *epoller, struct epoll_event *revent);

	/// @brief Called when signal is received.
	///
	/// Default implementation calls receiver::sighandler method of #rcvr if not null,
	/// otherwise calls #_sighandler if not null,
	/// otherwise returns -1.
	///
	/// @param siginfo structure with information about the received signal
	/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
	virtual int sighandler(struct signalfd_siginfo *siginfo);
};

#endif // SIGEPOLLER_H

