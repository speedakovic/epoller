/// @file   epoller/inotepoller.h
/// @author speedak
/// @brief  Inotify file descriptor wrapper.

#ifndef INOTEPOLLER_H
#define INOTEPOLLER_H

#include <epoller/epoller.h>
#include <sys/inotify.h>
#include <stdint.h>
#include <string>

/// @brief Inotify epoller.
struct inotepoller : public epoller_event
{
	int                fd;      ///< inotify file descriptor
	struct epoller    *epoller; ///< parent epoller
	struct epoll_event event;   ///< epoll event

	/// @brief Handler for inotify event.
	/// @see #inothandler
	/// @param inotepoller inotify epoller within that the event occurred
	int (*_inothandler) (struct inotepoller *inotepoller, struct inotify_event *event);

	/// @brief Constructor.
	/// @param epoller parent epoller
	inotepoller(struct epoller *epoller) : fd(-1), epoller(epoller), event(), _inothandler(0) {}

	/// @brief Destructor.
	virtual ~inotepoller() {cleanup();}

	/// @brief Initializes the inotify epoller.
	/// @return @c true if initialization was successful, otherwise @c false
	virtual bool init();

	/// @brief Cleanups the inotify epoller.
	virtual void cleanup();

	/// @brief Adds watch.
	/// @param pathname watched pathname
	/// @param mask mask with monitoring events (IN_OPEN, IN_MODIFY, IN_CLOSE_WRITE, ...)
	/// @return nonnegative watch descriptor if adding was successful, otherwise -1
	int add_watch(const std::string &pathname, uint32_t mask);

	/// @brief Removes watch.
	/// @param wd watch descriptor to be removed
	/// @return @c true if removing was successful, otherwise @c false
	bool rm_watch(int wd);

	/// @copydoc epoller_event::handler
	virtual int handler(struct epoller *epoller, struct epoll_event *revent);

	/// @brief Called when event occurs.
	///
	/// Default implementation calls #_inothandler if not null, otherwise returns 0.
	///
	/// @param event structure with information about the occurred event
	/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
	virtual int inothandler(struct inotify_event *event);
};

#endif // INOTEPOLLER_H

