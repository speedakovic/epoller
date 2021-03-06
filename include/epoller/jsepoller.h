/// @file   epoller/jsepoller.h
/// @author speedak
/// @brief  Joystick epoller.

#ifndef JSEPOLLER_H
#define JSEPOLLER_H

#include <epoller/fdepoller.h>
#include <linux/joystick.h>
#include <string>

/// @brief Joystick epoller.
struct jsepoller : fdepoller
{
	/// @brief Event receiver interface.
	struct receiver : virtual fdepoller::receiver
	{
		/// @brief Destructor.
		virtual ~receiver() {}

		/// @brief Called when joystick event is received.
		///        Default implementation returns -1.
		/// @param sender event sender
		/// @param event structure with information about the received joystick event
		/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
		virtual int jshandler(jsepoller &sender, struct js_event *event);
	};

	/// @brief Called when joystick event is received.
	/// @param sender event sender
	/// @param event structure with information about the received joystick event
	/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
	int (*_jshandler) (jsepoller &sender, struct js_event *event);

	/// @brief Constructor.
	/// @param epoller parent epoller
	jsepoller(struct epoller *epoller) : fdepoller(epoller) {}

	/// @brief Default constructor.
	jsepoller() : jsepoller(0) {}

	/// @brief Destructor.
	virtual ~jsepoller() {}

	/// @brief Opens joystick device.
	/// @param pathname joystick device
	/// @param evbuff length of event buffer in number of events
	/// @return @c true if opening was successful, otherwise @c false
	virtual bool open(const std::string &pathname, size_t evbuff = 1);

	/// @brief Gets number of axes.
	size_t get_axes();

	/// @brief Gets number of buttons.
	size_t get_buttons();

	/// @brief Gets version.
	int get_version();

	/// @brief Gets name.
	std::string get_name();

	/// @brief Gets corrections.
	/// @param corr array of correction structures, its length must be equal the number of axes
	/// @return zero if success, otherwise negative
	int get_corr(struct js_corr *corr);

	/// @brief Sets corrections.
	/// @param corr array of correction structures, its length must be equal the number of axes
	/// @return zero if success, otherwise negative
	int set_corr(const struct js_corr *corr);

	/// @copydoc fdepoller::rx
	virtual int rx(int len);

	/// @brief Called when joystick event is received.
	///
	/// Default implementation calls receiver::jshandler method of #rcvr if not null,
	/// otherwise calls #_jshandler if not null,
	/// otherwise returns -1.
	///
	/// @param event structure with information about the received joystick event
	/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
	virtual int jshandler(struct js_event *event);
};

#endif // JSEPOLLER_H

