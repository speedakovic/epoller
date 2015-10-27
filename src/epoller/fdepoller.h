/// @file   epoller/fdepoller.h
/// @author speedak
/// @brief  Generic file descriptor wrapper.

#ifndef FDEPOLLER_H
#define FDEPOLLER_H

#include <epoller/epoller.h>
#include <linbuff/linbuff.h>
#include <string>

/// @brief Generic file desciptor epoller.
struct fdepoller : public epoller_event
{
	int                fd;              ///< file descriptor
	struct epoller    *epoller;         ///< parent epoller
	struct epoll_event event;           ///< epoll event
	struct linbuff     rxbuff;          ///< rx buffer
	struct linbuff     txbuff;          ///< tx buffer
	bool               enabled;         ///< added to / removed from parent epoller
	bool               rx_auto_enable;  ///< auto rx enable flag
	bool               rx_auto_disable; ///< auto rx disable flag
	bool               tx_auto_enable;  ///< auto tx enable flag
	bool               tx_auto_disable; ///< auto tx disable flag
	unsigned long      epoll_in_cnt;    ///< EPOLLIN counter
	unsigned long      epoll_out_cnt;   ///< EPOLLOUT counter
	unsigned long      epoll_hup_cnt;   ///< EPOLLHUP counter
	unsigned long      epoll_err_cnt;   ///< EPOLLERR counter

	/// @brief Handler for rx events.
	/// @see #rx
	/// @param fdepoller file descriptor epoller within that the event occured
	int (*_rx) (struct fdepoller *fdepoller, int len);

	/// @brief Handler for tx events.
	/// @see #tx
	/// @param fdepoller file descriptor epoller within that the event occured
	int (*_tx) (struct fdepoller *fdepoller, int len);

	/// @brief Handler for hup events.
	/// @see #hup
	/// @param fdepoller file descriptor epoller within that the event occured
	int (*_hup) (struct fdepoller *fdepoller);

	/// @brief Handler for err events.
	/// @see #err
	/// @param fdepoller file descriptor epoller within that the event occured
	int (*_err) (struct fdepoller *fdepoller);

	/// @brief Handler for unknown events.
	/// @see #un
	/// @param fdepoller file descriptor epoller within that the event(s) occured
	int (*_un) (struct fdepoller *fdepoller, int events);

	/// @brief Handler for event handler enter.
	/// @see #enter
	/// @param fdepoller file descriptor epoller within that the event occured
	int (*_enter) (struct fdepoller *fdepoller, struct epoll_event *revent);

	/// @brief Handler for event handler exit.
	/// @see #exit
	/// @param fdepoller file descriptor epoller within that the event occured
	int (*_exit) (struct fdepoller *fdepoller, struct epoll_event *revent);

	/// @brief Constructor.
	/// @param epoller parent epoller
	fdepoller(struct epoller *epoller) :
	    fd              (-1     ),
	    epoller         (epoller),
	    event           (       ),
	    rxbuff          (       ),
	    txbuff          (       ),
	    enabled         (false  ),
	    rx_auto_enable  (true   ),
	    rx_auto_disable (true   ),
	    tx_auto_enable  (true   ),
	    tx_auto_disable (true   ),
	    epoll_in_cnt    (0      ),
	    epoll_out_cnt   (0      ),
	    epoll_hup_cnt   (0      ),
	    epoll_err_cnt   (0      ),
	    _rx             (0      ),
	    _tx             (0      ),
	    _hup            (0      ),
	    _err            (0      ),
	    _un             (0      ),
	    _enter          (0      ),
	    _exit           (0      )
	{}

	/// @brief Destructor.
	virtual ~fdepoller() {cleanup();}

	/// @brief Initializes the file descriptor epoller.
	///
	/// After successful initialization #fd, #event, #rxbuff, #txbuff,
	/// #epoll_in_cnt, #epoll_out_cnt, #epoll_hup_cnt and #epoll_err_cnt  members are initialized.
	///
	/// @param fd file descriptor
	/// @param rxsize size of rx buffer in bytes, allowed to be zero, but rxen must be @c false
	/// @param txsize size of tx buffer in bytes, allowed to be zero, but txen must be @c false
	/// @param rxen if @c true the reception will be enabled (by adding EPOLLIN event to epoller)
	/// @param txen if @c true the transmission will be enabled (by adding EPOLLOUT event to epoller)
	/// @param en if @c true the epoller will be enabled by adding to parent epoller
	///
	/// @return @c true if initialization was successful, otherwise @c false
	virtual bool init(int fd, size_t rxsize = 1024, size_t txsize = 1024, bool rxen = true, bool txen = false, bool en = true);

	/// @brief Cleanups the file descriptor epoller.
	///
	/// After cleanup #fd member is set to -1, #rxbuff and #txbuff members are freed.
	virtual void cleanup();

	/// @brief Opens file and initializes the file descriptor epoller.
	///
	/// @see #init
	///
	/// @param pathname name of the file to be open
	/// @param flags O_RDONLY, O_WRONLY, O_RDWR, ... (see doc for posix open)
	/// @param rxsize
	/// @param txsize
	/// @param rxen
	/// @param txen
	/// @param en
	///
	/// @return @c true if initialization was successful, otherwise @c false
	virtual bool open(std::string pathname, int flags, size_t rxsize = 1024, size_t txsize = 1024, bool rxen = true, bool txen = false, bool en = true);

	/// @brief Cleanups the file descriptor epoller and closes the file descriptor
	/// @see #cleanup
	virtual void close();

	/// @brief Enables the file descriptor epoller by adding to parent epoller.
	/// @param rxen if @c true the reception will be enabled (by adding EPOLLIN event to epoller)
	/// @param txen if @c true the transmission will be enabled (by adding EPOLLOUT event to epoller)
	/// @return @c true if enabling was successful, otherwise @c false
	bool enable(bool rxen = true, bool txen = false);

	/// @brief Disables the file descriptor epoller by removing from parent epoller.
	/// @return @c true if disabling was successful, otherwise @c false
	bool disable();

	/// @brief Disables reading by removing EPOLLIN event from epoller.
	/// @return @c true if disabling was successful, otherwise @c false
	bool disable_rx();

	/// @brief Enables reading by adding EPOLLIN event to epoller.
	/// @return @c true if enabling was successful, otherwise @c false
	bool enable_rx();

	/// @brief Disables writing by removing EPOLLOUT event from epoller.
	/// @return @c true if disabling was successful, otherwise @c false
	bool disable_tx();

	/// @brief Enables writing by adding EPOLLOUT event to epoller.
	/// @return @c true if enabling was successful, otherwise @c false
	bool enable_tx();

	/// @brief Sets file descriptor flags.
	///
	/// Only passed flags will be set, other ones will remain untouched.
	///
	/// @param flags flags to be set (O_APPEND, O_ASYNC, O_DIRECT, O_NOATIME, O_NONBLOCK)
	/// @return @c true if setting was successful, otherwise @c false
	bool set_flags(int flags);

	/// @brief Clears file descriptor flags.
	///
	/// Only passed flags will be cleared, other ones will remain untouched.
	///
	/// @param flags flags to be cleared (O_APPEND, O_ASYNC, O_DIRECT, O_NOATIME, O_NONBLOCK)
	/// @return @c true if clearing was successful, otherwise @c false
	bool clear_flags(int flags);

	/// @brief Gets file descriptor flags.
	/// @param flags
	/// @return @c true if getting was successful, otherwise @c false
	bool get_flags(int *flags);

	/// @brief Called if new data have just been received (to #rxbuff)
	/// or some error occurred during reception.
	///
	/// Default implementation calls #_rx if not null,
	/// otherwise returns 0 if len is positive, 1 if len is zero or -1 if len is negative.
	///
	/// @param len length of just received data if zero or positive, error state if negative
	/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
	virtual int rx(int len);

	/// @brief Called if some data have just been transmitted (from #txbuff)
	/// or some error occurred during transmission.
	///
	/// Default implementation calls #_tx if not null,
	/// otherwise returns 0 if len is positive, 1 if len is zero or -1 if len is negative.
	///
	/// @param len length of just transmitted data if zero or positive, error state if negative
	/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
	virtual int tx(int len);

	/// @brief Called if hang-out event occurred.
	///
	/// Default implementation calls #_hup if not null, otherwise returns -1.
	///
	/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
	virtual int hup();

	/// @brief Called if error event occurred.
	///
	/// Default implementation calls #_err if not null, otherwise returns -1.
	///
	/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
	virtual int err();

	/// @brief Called if unknown event occurred.
	///
	/// Default implementation calls #_un if not null, otherwise returns -1.
	///
	/// @param events unknown event flags
	/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
	virtual int un(int events);

	/// @brief Called when entering into epoller handler.
	///
	/// Default implementation calls #_enter if not null, otherwise returns 0.
	///
	/// @param revent event (as is returned from epoll_wait)
	/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
	virtual int enter(struct epoll_event *revent);

	/// @brief Called when exiting (with zero) from epoller handler.
	///
	/// Default implementation calls #_exit if not null, otherwise returns 0.
	///
	/// @param revent event (as is returned from epoll_wait)
	/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
	virtual int exit(struct epoll_event *revent);

	/// @brief EPOLLIN event handler.
	/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
	virtual int epoll_in();

	/// @brief EPOLLOUT event handler.
	/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
	virtual int epoll_out();

	/// @brief EPOLLHUP event handler.
	/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
	virtual int epoll_hup();

	/// @brief EPOLLERR event handler.
	/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
	virtual int epoll_err();

	/// @brief Unknown events handler.
	/// @param events unknown events
	/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
	virtual int epoll_unknown(int events);

	/// @copydoc epoller_event::handler
	virtual int handler(struct epoller *epoller, struct epoll_event *revent);
};

#endif // FDEPOLLER_H

