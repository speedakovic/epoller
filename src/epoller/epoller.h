/// @file   epoller/epoller.h
/// @author speedak
/// @brief  Epoll file descriptor wrapper.

#ifndef EPOLLER_H
#define EPOLLER_H

#include <cstddef>
#include <sys/epoll.h>

/// @brief Default number of events returned from epoll_wait.
#define EPOLLER_REVENTS_SIZE 1

/// @brief Epoller event.
///
/// Every epoll_event added to the epoller's epoll file descriptor must have set its epoll_data_t data member to the
/// object of type epoller_event.
struct epoller_event
{
	/// @brief Union holding some user data.
	union user
	{
		void         *ptr; ///< pointer
		int             i; ///< integer
		unsigned int   ui; ///< unsigned integer
		long            l; ///< long
		unsigned long  ul; ///< unsigned long

		user() {ptr = 0; i = 0; ui = 0; l = 0; ul = 0;}

	} user; ///< holder for user data

	struct epoller_event **pthis; ///< pointer to this pointer, controlled by epoller

	/// @brief Constructor.
	epoller_event() : user(), pthis(0) {}

	/// @brief Destructor.
	virtual ~epoller_event() {if (pthis) *pthis = 0;}

	/// @brief Called when an event occurs.
	/// @param epoller epoller within that the event occured
	/// @param revent ocurred event
	/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
	virtual int handler(struct epoller *epoller, struct epoll_event *revent) = 0;

};

/// @brief Epoll wrapper.
struct epoller
{
	int                 fd;           ///< epoll file descriptor
	int                 timeout;      ///< epoll timeout in milliseconds (-1 for block indefinitely, 0 for return immediately)
	int                 loop_exit;    ///< zero for loop continuation, positive for normal loop exit, negative for loop exit with error
	size_t              revents_size; ///< size of array for returned events
	struct epoll_event *revents;      ///< array for returned events

	/// @brief Called when epoll timeout occurs.
	/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
	int (*timeout_handler) (struct epoller *epoller);

	/// @brief Called just before epoll_wait entry.
	/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
	int (*pre_epoll_handler) (struct epoller *epoller);

	/// @brief Called just after epoll_wait return.
	/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
	int (*post_epoll_handler) (struct epoller *epoller);

	/// @brief Called after epoll_wait return with non-zero number of events.
	/// @param revents array containing occurred events (as are returned from epoll_wait)
	/// @param revents_count number of events contained in revents array
	/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
	int (*revents_handler) (struct epoller *epoller, struct epoll_event *revents, size_t revents_count);

	/// @brief Constructor.
	epoller() :
	    fd                (-1                                   ),
	    timeout           (-1                                   ),
	    loop_exit         ( 0                                   ),
	    revents_size      (EPOLLER_REVENTS_SIZE                 ),
	    revents           (new epoll_event[EPOLLER_REVENTS_SIZE]),
	    timeout_handler   ( 0                                   ),
	    pre_epoll_handler ( 0                                   ),
	    post_epoll_handler( 0                                   ),
	    revents_handler   ( 0                                   )
	{}

	/// @brief Constructor.
	/// @param revents_size maximum number of events returned from epoll_wait.
	epoller(size_t revents_size) :
	    fd                (-1                           ),
	    timeout           (-1                           ),
	    loop_exit         ( 0                           ),
	    revents_size      (revents_size                 ),
	    revents           (new epoll_event[revents_size]),
	    timeout_handler   ( 0                           ),
	    pre_epoll_handler ( 0                           ),
	    post_epoll_handler( 0                           ),
	    revents_handler   ( 0                           )
	{}

	/// @brief Destructor.
	virtual ~epoller() {delete [] revents;}

	/// @brief Initializes the epoller.
	///
	/// Only after this method successful returns the epoll file descriptor is created and so epoll events can be added,
	/// modified or deleted.
	///
	/// @return @c true if initialization was successful, otherwise @c false
	virtual bool init();

	/// @brief Cleanups the epoller.
	virtual void cleanup();

	/// @brief Runs the epoller's looper.
	/// @return @c true for normal exit, @c false for exit caused by some error
	virtual bool loop();

	/// @brief Exits from the epoller's looper.
	/// @param how positive for normal loop exit, negative for loop exit with error
	virtual void exit(int how);
};

#endif // EPOLLER_H

