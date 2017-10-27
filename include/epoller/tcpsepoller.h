/// @file   epoller/tcpsepoller.h
/// @author speedak
/// @brief  TCP server epoller.

#ifndef TCPSEPOLLER_H
#define TCPSEPOLLER_H

#include <epoller/sockepoller.h>

/// @brief TCP server based on socket epoller.
struct tcpsepoller : public sockepoller
{
	/// @brief Event receiver interface.
	struct receiver : public fdepoller::receiver
	{
		/// @brief Destructor.
		virtual ~receiver() {}

		/// @brief Called if accepting is done.
		/// @param sender event sender
		/// @param fd non-negative file descriptor of accepted socket or -1 indicating error with errno set appropriately
		/// @param addr peer socket address
		/// @param addrlen size of peer socket address
		/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
		virtual int acc(tcpsepoller &sender, int fd, const struct sockaddr *addr, const socklen_t *addrlen) = 0;
	};

	/// @brief Connecting result handler.
	/// @see #acc
	/// @param tcpsepoller TCP server epoller within that the event occured
	int (*_acc) (struct tcpsepoller *tcpsepoller, int fd, const struct sockaddr *addr, const socklen_t *addrlen);

	/// @brief Constructor.
	/// @param epoller parent epoller
	tcpsepoller(struct epoller *epoller) : sockepoller(epoller), _acc(0) {}

	/// @brief Default constructor.
	tcpsepoller() : tcpsepoller(0) {}

	/// @brief Creates listening socket.
	/// @param domain is passed to the posix socket function (AF_INET, AF_INET6, ...)
	/// @param ip ip address (IPv4/IPv6) the socket should be bound to, when empty @c INADDR_ANY  or @c in6addr_any will be used
	/// @param port port number the socket should be bound to
	/// @param backlog maximum number of pending connections (before they are refused)
	/// @param reuseaddr @c true if address reusing should be enabled, otherwise @c false
	/// @return @c true if socket was created successfully, otherwise @c false
	virtual bool socket(int domain, const std::string &ip, unsigned short port, int backlog = 1, bool reuseaddr = true);

	/// @brief Called if accepting is done.
	///
	/// Default implementation calls receiver::acc method of #rcvr if not null,
	/// otherwise calls #_acc if not null,
	/// otherwise returns 0.
	///
	/// @param fd non-negative file descriptor of accepted socket or -1 indicating error with errno set appropriately
	/// @param addr peer socket address
	/// @param addrlen size of peer socket address
	/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
	virtual int acc(int fd, const struct sockaddr *addr, const socklen_t *addrlen);

	/// @see sockepoller::epoll_in
	virtual int epoll_in();
};

#endif // TCPSEPOLLER_H

