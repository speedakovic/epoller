/// @file   epoller/tcpcepoller.h
/// @author speedak
/// @brief  TCP client epoller.

#ifndef TCPCEPOLLER_H
#define TCPCEPOLLER_H

#include <epoller/sockepoller.h>

/// @brief TCP client based on socket epoller.
struct tcpcepoller : public sockepoller
{
	bool connecting; ///< connecting state flag

	/// @brief Connecting result handler.
	/// @see #cdone
	/// @param tcpcepoller TCP client epoller within that the event occured
	int (*_cdone) (struct tcpcepoller *tcpcepoller, bool connected);

	/// @brief Constructor.
	/// @param epoller parent epoller
	tcpcepoller(struct epoller *epoller) : sockepoller(epoller), connecting(false), _cdone(0) {}

	/// @brief Default constructor.
	tcpcepoller() : tcpcepoller(0) {}

	/// @brief Creates socket and initializes the socket epoller of given parameters.
	///        Created socket is disabled (not present in parent epoller).
	///
	/// @param domain is passed to the posix socket function (AF_INET, AF_INET6, ...)
	/// @param rxsize
	/// @param txsize
	///
	/// @return @c true if socket was created successfully, otherwise @c false
	virtual bool socket(int domain, size_t rxsize = 1024, size_t txsize = 1024);

	/// @brief Closes socket.
	virtual void close();

	/// @brief Starts connecting.
	///        Socket in connecting state has enabled only EPOLLOUT events.
	///        Connecting result will be announced through #cdone method.
	///        In case of any serious error any of #hup, #err or even #un methods may be called.
	///        No #tx method will be called in connecting state as EPOLLOUT event is fully consumed by #epoll_out handler.
	///        Neither #rx nor #pri methods will be called as these events are not enabled.
	///
	/// @param ip ip address (IPv4/IPv6) the socket should be connected to
	/// @param port port number the socket should be connected to
	/// @return @c true if connecting was successfully started, otherwise @c false
	virtual bool connect(const std::string &ip, unsigned short port);

	/// @brief Called if connecting is done.
	///        After connecting is done (whatever result), socket has enabled only EPOLLOUT events.
	///        In case of any serious error any of #hup, #err or even #un methods may be called.
	///        No #tx method will be called as the very first EPOLLOUT event was fully consumed by #epoll_out handler.
	///        Neither #rx nor #pri methods will be called as these events are not enabled.
	///
	/// @param connected @c true if socket is connected, otherwise @c false
	/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
	virtual int cdone(bool connected);

	/// @see sockepoller::epoll_out
	virtual int epoll_out();

	/// @see sockepoller::epoll_hup
	virtual int epoll_hup();

	/// @see sockepoller::epoll_err
	virtual int epoll_err();

	/// @see sockepoller::epoll_unknown
	virtual int epoll_unknown(int events);
};

#endif // TCPCEPOLLER_H

