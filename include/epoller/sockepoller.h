/// @file   epoller/sockepoller.h
/// @author speedak
/// @brief  Socket file descriptor wrapper.

#ifndef SOCKEPOLLER_H
#define SOCKEPOLLER_H

#include <epoller/fdepoller.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <string>
#include <iostream>

/// @brief General socket epoller based on file descriptor epoller.
struct sockepoller : public fdepoller
{
	int rx_flags; ///< flags passed to recv
	int tx_flags; ///< flags passed to send

	/// @brief Constructor.
	/// @param epoller parent epoller
	sockepoller(struct epoller *epoller) : fdepoller(epoller), rx_flags(0), tx_flags(0) {}

	/// @brief Default constructor.
	sockepoller() : sockepoller(0) {}

	/// @brief Initializes the socket epoller.
	///
	/// @param fd
	/// @param rxsize
	/// @param txsize
	/// @param rxen
	/// @param txen
	/// @param en
	///
	/// @see fdepoller::init
	virtual bool init(int fd, size_t rxsize = 1024, size_t txsize = 1024, bool rxen = true, bool txen = false, bool en = true);

	/// @brief Creates socket and initializes the socket epoller of given parameters.
	///
	/// @param domain is passed to the posix socket function (AF_INET, AF_INET6, ...)
	/// @param type is passed to the posix socket function (SOCK_STREAM, SOCK_DGRAM, ...)
	/// @param protocol is passed to the posix socket function
	/// @param rxsize
	/// @param txsize
	/// @param rxen
	/// @param txen
	/// @param en
	///
	/// @see fdepoller::init
	virtual bool socket(int domain, int type, int protocol, size_t rxsize = 1024, size_t txsize = 1024, bool rxen = true, bool txen = false, bool en = true);

	/// @brief Shutdowns the socket.
	///
	/// For TCP socket:
	///
	/// SHUT_RDWR, SHUT_WR - four-way handshake is performed, hup and zero length rx events are emitted
	/// SHUT_RD - zero length rx event is emitted
	///
	/// @param how SHUT_RD, SHUT_WR, SHUT_RDWR
	/// @return @c true if shutdown was successful, otherwise @c false
	virtual bool shutdown(int how = SHUT_RDWR);

	/// @brief Turns socket into pasive state.
	/// @param backlog maximum number of pending connections (before they are refused)
	/// @return @c true if listen was successful, otherwise @c false
	virtual bool listen(int backlog = 1);

	/// @brief Binds socket of AF_INET/AF_INET6 domain.
	/// @param ip ip address (IPv4/IPv6) the socket should be bound to, when empty @c INADDR_ANY  or @c in6addr_any will be used
	/// @param port port number the socket should be bound to
	/// @return @c true if socket was successfully bound, otherwise @c false
	virtual bool bind(const std::string &ip, unsigned short port);

	/// @brief Accepts connection request.
	/// @param addr address of the peer socket (allowed to be NULL)
	/// @param addrlen must be initialized with size of space pointed by addr argument, on return it will
	///                contain actual size of the peer socket (allowed to be NULL if addr argument is NULL as well)
	/// @return non-negative socket descriptor or -1 if accepting failed
	virtual int accept(struct sockaddr *addr, socklen_t *addrlen);

	/// @brief Connects socket of AF_INET/AF_INET6 domain.
	///
	/// For TCP socket:
	///
	/// The first tx event with zero length signalises, that the connection is established.
	/// The first tx event with zegative length signalizes, that the connection establishing failed.
	/// The first rx event with negative length signalises, that the connection establishing failed.
	///
	/// @param ip ip address (IPv4/IPv6) the socket should be connected to
	/// @param port port number the socket should be connected to
	/// @return @c true if socket was successfully connected, otherwise @c false
	virtual bool connect(const std::string &ip, unsigned short port);

	/// @brief Gets socket domain.
	/// @param domain socket domain (AF_INET, AF_INET6, AF_UNIX, ...)
	/// @return @c true if getting was successful, otherwise @c false
	bool get_so_domain(int *domain);

	/// @brief Gets socket type.
	/// @param type socket type (SOCK_STREAM, SOCK_DGRAM, SOCK_RAW, ...)
	/// @return @c true if getting was successful, otherwise @c false
	bool get_so_type(int *type);

	/// @brief Gets socket error.
	/// @param error error code or zero
	/// @return @c true if getting was successful, otherwise @c false
	bool get_so_error(int *error);

	/// @brief Sets SO_SNDBUF socket option.
	/// @param value size of send buffer in bytes
	/// @return @c true if setting was successful, otherwise @c false
	bool set_so_sndbuf(int value);

	/// @brief Gets SO_SNDBUF socket option.
	/// @param value
	/// @return @c true if getting was successful, otherwise @c false
	bool get_so_sndbuf(int *value);

	/// @brief Sets SO_RECVBUF socket option.
	/// @param value size of recv buffer in bytes
	/// @return @c true if setting was successful, otherwise @c false
	bool set_so_rcvbuf(int value);

	/// @brief Gets SO_RECVBUF socket option.
	/// @param value
	/// @return @c true if getting was successful, otherwise @c false
	bool get_so_rcvbuf(int *value);

	/// @brief Sets SO_LINGER socket option.
	/// @param enabled @c true if linger should be enabled, otherwise @c false
	/// @param timeout timeout in seconds
	/// @return @c true if setting was successful, otherwise @c false
	bool set_so_linger(bool enabled, int timeout);

	/// @brief Gets SO_LINGER socket option.
	/// @param enabled
	/// @param timeout
	/// @return @c true if getting was successful, otherwise @c false
	bool get_so_linger(bool *enabled, int *timeout);

	/// @brief Sets SO_REUSEADDR socket option.
	/// @param enabled @c true if address reusing should be enabled, otherwise @c false
	/// @return @c true if setting was successful, otherwise @c false
	bool set_so_reuseaddr(bool enabled);

	/// @brief Gets SO_REUSEADDR socket option.
	/// @param enabled
	/// @return @c true if getting was successful, otherwise @c false
	bool get_so_reuseaddr(bool *enabled);

	/// @brief Sets SO_KEEPALIVE socket option.
	/// @param enabled @c true if keepalive feature should be enabled, otherwise @c false
	/// @return @c true if setting was successful, otherwise @c false
	bool set_so_keepalive(bool enabled);

	/// @brief Gets SO_KEEPALIVE socket option.
	/// @param enabled
	/// @return @c true if getting was successful, otherwise @c false
	bool get_so_keepalive(bool *enabled);

	/// @brief Sets SO_BROADCAST socket option.
	/// @param enabled @c true if broadcast feature (only for SOCK_DGRAM sockets) should be enabled, otherwise @c false
	/// @return @c true if setting was successful, otherwise @c false
	bool set_so_broadcast(bool enabled);

	/// @brief Gets SO_BROADCAST socket option.
	/// @param enabled
	/// @return @c true if getting was successful, otherwise @c false
	bool get_so_broadcast(bool *enabled);

	/// @brief Sets SO_BINDTODEVICE socket option.
	/// @param dev name of network device (eth0, eth1, ...) the socket should be bound to
	/// @return @c true if setting was successful, otherwise @c false
	bool set_so_bindtodevice(const std::string &dev);

	/// @brief Gets SO_BINDTODEVICE socket option.
	/// @param dev
	/// @return @c true if getting was successful, otherwise @c false
	bool get_so_bindtodevice(std::string *dev);

	/// @brief Sets SO_RCVTIMEO socket option.
	/// @param timeout
	/// @return @c true if setting was successful, otherwise @c false
	bool set_so_rcvtimeo(const struct timeval *timeout);

	/// @brief Gets SO_RCVTIMEO socket option.
	/// @param timeout
	/// @return @c true if setting was successful, otherwise @c false
	bool get_so_rcvtimeo(struct timeval *timeout);

	/// @brief Sets SO_SNDTIMEO socket option.
	/// @param timeout
	/// @return @c true if setting was successful, otherwise @c false
	bool set_so_sndtimeo(const struct timeval *timeout);

	/// @brief Gets SO_SNDTIMEO socket option.
	/// @param timeout
	/// @return @c true if setting was successful, otherwise @c false
	bool get_so_sndtimeo(struct timeval *timeout);

	/// @brief Sets TCP_NODELAY socket option.
	/// @param enabled @c true if tcp nodelay (no Nagle algorithm => no data accumulation (untill ack)) feature should be enabled, otherwise @c false
	/// @return @c true if setting was successful, otherwise @c false
	bool set_so_tcp_nodelay(bool enabled);

	/// @brief Gets TCP_NODELAY socket option.
	/// @param enabled
	/// @return @c true if getting was successful, otherwise @c false
	bool get_so_tcp_nodelay(bool *enabled);

	/// @brief Sets TCP_CORK socket option.
	/// @param enabled @c true if tcp cork (data accumulation (untill buff fill)) feature should be enabled, otherwise @c false
	/// @return @c true if setting was successful, otherwise @c false
	bool set_so_tcp_cork(bool enabled);

	/// @brief Gets TCP_CORK socket option.
	/// @param enabled
	/// @return @c true if getting was successful, otherwise @c false
	bool get_so_tcp_cork(bool *enabled);

	/// @brief Sets TCP_KEEPIDLE socket option.
	///
	/// The time (in seconds) the connection needs to remain idle before tcp starts sending
	/// keepalive probes, if the socket option SO_KEEPALIVE has been set on this socket.
	///
	/// @param value time in seconds
	/// @return @c true if setting was successful, otherwise @c false
	bool set_so_tcp_keepidle(int value);

	/// @brief Gets TCP_KEEPIDLE socket option.
	/// @param value
	/// @return @c true if getting was successful, otherwise @c false
	bool get_so_tcp_keepidle(int *value);

	/// @brief Sets TCP_KEEPINTVL socket option.
	///
	/// The time (in seconds) between individual keepalive probes.
	///
	/// @param value time in seconds
	/// @return @c true if setting was successful, otherwise @c false
	bool set_so_tcp_keepintvl(int value);

	/// @brief Gets TCP_KEEPINTVL socket option.
	/// @param value
	/// @return @c true if getting was successful, otherwise @c false
	bool get_so_tcp_keepintvl(int *value);

	/// @brief Sets TCP_KEEPCNT socket option.
	///
	/// The maximum number of keepalive probes tcp should send before dropping the connection.
	///
	/// @param value number of probes
	/// @return @c true if setting was successful, otherwise @c false
	bool set_so_tcp_keepcnt(int value);

	/// @brief Gets TCP_KEEPCNT socket option.
	/// @param value
	/// @return @c true if getting was successful, otherwise @c false
	bool get_so_tcp_keepcnt(int *value);

	/// @brief Sets TCP_MAXSEG socket option.
	/// @param value maximum segment size in bytes
	/// @return @c true if setting was successful, otherwise @c false
	bool set_so_tcp_maxseg(int value);

	/// @brief Gets TCP_MAXSEG socket option.
	/// @param value
	/// @return @c true if getting was successful, otherwise @c false
	bool get_so_tcp_maxseg(int *value);

	/// @brief Sets TCP_SYNCNT socket option.
	/// @param value number of SYN retransmits
	/// @return @c true if setting was successful, otherwise @c false
	bool set_so_tcp_syncnt(int value);

	/// @brief Gets TCP_SYNCNT socket option.
	/// @param value
	/// @return @c true if getting was successful, otherwise @c false
	bool get_so_tcp_syncnt(int *value);

	/// @brief Gets TCP_INFO socket option.
	/// @param tcp_info pointer to structure where the info should be stored, see include/tcp.h
	/// @return @c true if getting was successful, otherwise @c false
	bool get_so_tcp_info(struct tcp_info *tcp_info);

	/// @brief Fills sockaddr_storage struct (only for sockets of AF_INET/AFINET6 domain).
	/// @param ip IPv4/IPv6 address, when empty @c INADDR_ANY/in6addr_any will be used
	/// @param port port
	/// @param addr address struct to be filled (should be zeroed before usage of this method)
	/// @param addr_len address struct length to be filled
	/// @return @c true if filling was successful, otherwise @c false
	bool fill_addr_inet(const std::string &ip, unsigned short port, struct sockaddr_storage *addr, socklen_t *addr_len);

	/// @brief Does the same as fdepoller::epoll_in, but uses recv filled with #rx_flags.
	/// @see fdepoller::epoll_in
	virtual int epoll_in();

	/// @brief Does the same as fdepoller::epoll_out, but uses send filled with #tx_flags.
	/// @see fdepoller::epoll_out
	virtual int epoll_out();

	/// @brief Prints tcp info to output stream.
	/// @param tcp_info
	/// @param out
	static void print_tcp_info(const struct tcp_info *tcp_info, std::ostream &out = std::cout);

	/// @brief Converts tcp state (struct tcp_info::tcpi_state) to string.
	static std::string tcp_state_2_str(int state);

	/// @brief Converts socket domain to string.
	static std::string so_domain_2_str(int domain);

	/// @brief Converts socket type to string.
	static std::string so_type_2_str(int type);

	/// @brief Fills sockaddr_in struct.
	/// @param ip IPv4 address, when empty @c INADDR_ANY will be used
	/// @param port port
	/// @param addr address struct to be filled
	/// @return @c true if filling was successful, otherwise @c false
	static bool fill_addr_inet4(const std::string &ip, unsigned short port, struct sockaddr_in *addr);

	/// @brief Fills sockaddr_in6 struct.
	/// @param ip IPv6 address, when empty @c in6addr_any will be used
	/// @param port port
	/// @param addr address struct to be filled
	/// @return @c true if filling was successful, otherwise @c false
	static bool fill_addr_inet6(const std::string &ip, unsigned short port, struct sockaddr_in6 *addr);

};

#endif // SOCKEPOLLER_H

