/// @file   epoller/gepoller.h
/// @author speedak
/// @brief  Epoll file descriptor wrapper extended by processing of glib main context.

#ifndef GEPOLLER_H
#define GEPOLLER_H

#include <epoller/epoller.h>
#include <glib.h>

/// @brief Default number of processed glib file descriptors.
#define GEPOLLER_GEVENTS_SIZE 32

/// @brief Glib file descriptor wrapper.
///        Only for internal usage.
struct gepoller_event : epoller_event
{
	struct epoll_event  event; ///< epoll event
	GPollFD            *gfd;   ///< glib file descriptor

	/// @brief Constructor.
	gepoller_event() : event(), gfd(0) {}

	/// @brief Destructor.
	virtual ~gepoller_event() {}

	/// @copydoc epoller_event::handler
	virtual int handler(struct epoller *epoller, struct epoll_event *revent);

	/// @brief Adds file descriptor of this event to given epoller.
	///        #gfd member must be initialized
	bool add_to_epoller(struct epoller *epoller);

	/// @brief Deletes file descriptor of this event from given epoller.
	///        #gfd member must be initialized
	bool del_from_epoller(struct epoller *epoller);

	/// @brief Converts event flags from glib to linux epoll.
	inline static int events_glib2epoll(int events);

	/// @brief Converts event flags from linux epoll to glib.
	inline static int events_epoll2glib(int events);
};

/// @brief Epoll wrapper with glib main context processing support.
struct gepoller : epoller
{
	size_t          gevents_size; ///< size of gepoller events (and also glib file descriptors) array
	gepoller_event *gevents;      ///< gepoller events
	GPollFD        *gfds;         ///< glib file descriptors
	GMainContext   *context;      ///< glib main context

	/// @brief Constructor.
	gepoller() :
	    epoller     (                                ),
	    gevents_size(GEPOLLER_GEVENTS_SIZE           ),
	    gevents     (new gepoller_event[gevents_size]),
	    gfds        (new GPollFD[gevents_size]       ),
	    context     (0                               )
	{
		set_context(g_main_context_default());
	}

	/// @brief Constructor.
	/// @param revents_size maximum number of events returned from epoll_wait.
	/// @param gevents_size maximum number of processed glib file descriptors.
	gepoller(size_t revents_size, size_t gevents_size) :
	    epoller     (revents_size                    ),
	    gevents_size(gevents_size                    ),
	    gevents     (new gepoller_event[gevents_size]),
	    gfds        (new GPollFD[gevents_size]       ),
	    context     (0                               )
	{
		set_context(g_main_context_default());
	}

	/// @brief Destructor.
	virtual ~gepoller() {
		set_context(0);
		delete [] gevents;
		delete [] gfds;
	}

	/// @copydoc epoller::loop
	virtual bool loop();

	/// @brief Sets glib main context iterated within the loop.
	///        May be called only before loop is called.
	void set_context(GMainContext *context);

	/// @brief Adds glib file descriptors to epoller.
	///        Only for internal usage.
	bool add_gevents_to_epoller(int gfds_n);

	/// @brief Deletes glib file descriptors from epoller.
	///        Only for internal usage.
	bool del_gevents_from_epoller(int gfds_n);
};

#endif // GEPOLLER_H

