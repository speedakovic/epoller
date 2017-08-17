#include <epoller/sockepoller.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>
#include <iostream>
#include <cerrno>

#define DBG_PREFIX "sockepoller: "

bool sockepoller::init(int fd, size_t rxsize, size_t txsize, bool rxen, bool txen, bool en)
{
	if (this->fd != -1)
		return true;

	rx_flags = 0;
	tx_flags = 0;

	if(!fdepoller::init(fd, rxsize, txsize, rxen, txen, en))
		return false;

	return true;
}

bool sockepoller::socket(int domain, int type, int protocol, size_t rxsize, size_t txsize, bool rxen, bool txen, bool en)
{
	if (this->fd != -1)
		return true;

	int fd = ::socket(domain, type, protocol);
	if (fd == -1) {
		perror(DBG_PREFIX"creating socket failed");
		return false;
	}

	if(!init(fd, rxsize, txsize, rxen, txen, en)) {
		::close(fd);
		return false;
	}

	return true;
}

bool sockepoller::shutdown(int how)
{
	int ret = ::shutdown(fd, how);
	if (ret == -1) {
		perror(DBG_PREFIX"shutdowning socket failed");
		return false;
	}
	return true;
}

bool sockepoller::listen(int backlog)
{
	int ret = ::listen(fd, backlog);
	if (ret == -1) {
		perror(DBG_PREFIX"setting socket into passive state failed");
		return false;
	}
	return true;
}

bool sockepoller::bind(const std::string &ip, unsigned short port)
{
	int ret;
	struct sockaddr_storage addr = {};
	socklen_t addr_len;

	if (!fill_addr_inet(ip, port, &addr, &addr_len))
		return false;

	ret = ::bind(fd, (const struct sockaddr *) &addr, addr_len);
	if (ret == -1) {
		perror(DBG_PREFIX"binding socket epoller failed");
		return false;
	}

	return true;
}

int sockepoller::accept(struct sockaddr *addr, socklen_t *addrlen)
{
	int new_fd = ::accept(fd, addr, addrlen);
	if (new_fd == -1) {
		perror(DBG_PREFIX"accepting on socket epoller failed");
		return -1;
	}

	return new_fd;
}

bool sockepoller::connect(const std::string &ip, unsigned short port)
{
	int ret, flags;
	struct sockaddr_storage addr = {};
	socklen_t addr_len;

	if (!fill_addr_inet(ip, port, &addr, &addr_len))
		return false;

	if (!get_flags(&flags))
		return false;

	ret = ::connect(fd, (const struct sockaddr *) &addr, addr_len);
	if (flags & O_NONBLOCK) {
		if (ret == -1 && errno != EINPROGRESS) {
			perror(DBG_PREFIX"connecting non-blocking socket failed");
			return false;
		}
	} else {
		if (ret == -1) {
			perror(DBG_PREFIX"connecting blocking socket epoller failed");
			return false;
		}
	}

	return true;
}

bool sockepoller::get_so_domain(int *domain)
{
	socklen_t len = sizeof(int);

	if (getsockopt(fd, SOL_SOCKET, SO_DOMAIN, domain, &len) == -1) {
		perror(DBG_PREFIX"getting SO_DOMAIN failed");
		return false;
	}

	if (len != sizeof(int)) {
		std::cerr << DBG_PREFIX"getting SO_DOMAIN failed, wrong length returned" << std::endl;
		return false;
	}

	return true;
}

bool sockepoller::get_so_type(int *type)
{
	socklen_t len = sizeof(int);

	if (getsockopt(fd, SOL_SOCKET, SO_TYPE, type, &len) == -1) {
		perror(DBG_PREFIX"getting SO_TYPE failed");
		return false;
	}

	if (len != sizeof(int)) {
		std::cerr << DBG_PREFIX"getting SO_TYPE failed, wrong length returned" << std::endl;
		return false;
	}

	return true;
}

bool sockepoller::get_so_error(int *error)
{
	socklen_t len = sizeof(int);

	if (getsockopt(fd, SOL_SOCKET, SO_ERROR, error, &len) == -1) {
		perror(DBG_PREFIX"getting SO_ERROR failed");
		return false;
	}

	if (len != sizeof(int)) {
		std::cerr << DBG_PREFIX"getting SO_ERROR failed, wrong length returned" << std::endl;
		return false;
	}

	return true;
}

bool sockepoller::set_so_sndbuf(int value)
{
	if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &value, sizeof value) == -1) {
		perror(DBG_PREFIX"setting SO_SNDBUF failed");
		return false;
	}

	return true;
}

bool sockepoller::get_so_sndbuf(int *value)
{
	socklen_t len = sizeof(int);

	if (getsockopt(fd, SOL_SOCKET, SO_SNDBUF, value, &len) == -1) {
		perror(DBG_PREFIX"getting SO_SNDBUF failed");
		return false;
	}

	if (len != sizeof(int)) {
		std::cerr << DBG_PREFIX"getting SO_SNDBUF failed, wrong length returned" << std::endl;
		return false;
	}

	return true;
}

bool sockepoller::set_so_rcvbuf(int value)
{
	if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &value, sizeof value) == -1) {
		perror(DBG_PREFIX"setting SO_RCVBUF failed");
		return false;
	}

	return true;
}

bool sockepoller::get_so_rcvbuf(int *value)
{
	socklen_t len = sizeof(int);

	if (getsockopt(fd, SOL_SOCKET, SO_RCVBUF, value, &len) == -1) {
		perror(DBG_PREFIX"getting SO_RCVBUF failed");
		return false;
	}

	if (len != sizeof(int)) {
		std::cerr << DBG_PREFIX"getting SO_RCVBUF failed, wrong length returned" << std::endl;
		return false;
	}

	return true;
}

bool sockepoller::set_so_linger(bool enabled, int timeout)
{
	struct linger linger = {};

	linger.l_onoff = enabled;
	linger.l_linger = timeout;
	if (setsockopt(fd, SOL_SOCKET, SO_LINGER, &linger, sizeof linger) == -1) {
		perror(DBG_PREFIX"setting SO_LINGER failed");
		return false;
	}

	return true;
}

bool sockepoller::get_so_linger(bool *enabled, int *timeout)
{
	struct linger linger = {};
	socklen_t len = sizeof linger;

	if (getsockopt(fd, SOL_SOCKET, SO_LINGER, &linger, &len) == -1) {
		perror(DBG_PREFIX"getting SO_LINGER failed");
		return false;
	}

	if (len != sizeof linger) {
		std::cerr << DBG_PREFIX"getting SO_LINGER failed, wrong length returned" << std::endl;
		return false;
	}

	*enabled = linger.l_onoff;
	*timeout = linger.l_linger;

	return true;
}

bool sockepoller::set_so_reuseaddr(bool enabled)
{
	int reuse = enabled;

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof reuse) == -1) {
		perror(DBG_PREFIX"setting SO_REUSEADDR failed");
		return false;
	}

	return true;
}

bool sockepoller::get_so_reuseaddr(bool *enabled)
{
	int reuse;
	socklen_t len = sizeof reuse;

	if (getsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, &len) == -1) {
		perror(DBG_PREFIX"getting SO_REUSEADDR failed");
		return false;
	}

	if (len != sizeof reuse) {
		std::cerr << DBG_PREFIX"getting SO_REUSEADDR failed, wrong length returned" << std::endl;
		return false;
	}

	*enabled = reuse;

	return true;
}

bool sockepoller::set_so_keepalive(bool enabled)
{
	int keep = enabled;

	if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &keep, sizeof keep) == -1) {
		perror(DBG_PREFIX"setting SO_KEEPALIVE failed");
		return false;
	}

	return true;
}

bool sockepoller::get_so_keepalive(bool *enabled)
{
	int keep;
	socklen_t len = sizeof keep;

	if (getsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &keep, &len) == -1) {
		perror(DBG_PREFIX"getting SO_KEEPALIVE failed");
		return false;
	}

	if (len != sizeof keep) {
		std::cerr << DBG_PREFIX"getting SO_KEEPALIVE failed, wrong length returned" << std::endl;
		return false;
	}

	*enabled = keep;

	return true;
}

bool sockepoller::set_so_broadcast(bool enabled)
{
	int broadcast = enabled;

	if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast) == -1) {
		perror(DBG_PREFIX"setting SO_BROADCAST failed");
		return false;
	}

	return true;
}

bool sockepoller::get_so_broadcast(bool *enabled)
{
	int broadcast;
	socklen_t len = sizeof broadcast;

	if (getsockopt(fd, SOL_SOCKET, SO_BROADCAST, &broadcast, &len) == -1) {
		perror(DBG_PREFIX"getting SO_BROADCAST failed");
		return false;
	}

	if (len != sizeof broadcast) {
		std::cerr << DBG_PREFIX"getting SO_BROADCAST failed, wrong length returned" << std::endl;
		return false;
	}

	*enabled = broadcast;

	return true;
}

bool sockepoller::set_so_bindtodevice(const std::string &dev)
{
	if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, dev.c_str(), dev.length()) == -1) {
		perror(DBG_PREFIX"setting SO_BINDTODEVICE failed");
		return false;
	}

	return true;
}

bool sockepoller::get_so_bindtodevice(std::string *dev)
{
	char _dev[IFNAMSIZ + 1] = {};
	socklen_t len = IFNAMSIZ;

	if (getsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, _dev, &len) == -1) {
		perror(DBG_PREFIX"getting SO_BINDTODEVICE failed");
		return false;
	}

	_dev[len] = 0;
	*dev = std::string(_dev);

	return true;
}

bool sockepoller::set_so_rcvtimeo(const struct timeval *timeout)
{
	if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, timeout, sizeof(struct timeval)) == -1) {
		perror(DBG_PREFIX"setting SO_RCVTIMEO failed");
		return false;
	}

	return true;
}

bool sockepoller::get_so_rcvtimeo(struct timeval *timeout)
{
	socklen_t len = sizeof(struct timeval);
	if (getsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, timeout, &len) == -1) {
		perror(DBG_PREFIX"getting SO_RCVTIMEO failed");
		return false;
	}

	if (len != sizeof(struct timeval)) {
		std::cerr << DBG_PREFIX"getting SO_RCVTIMEO failed, wrong length returned" << std::endl;
		return false;
	}

	return true;
}

bool sockepoller::set_so_sndtimeo(const struct timeval *timeout)
{
	if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, timeout, sizeof(struct timeval)) == -1) {
		perror(DBG_PREFIX"setting SO_SNDTIMEO failed");
		return false;
	}

	return true;
}

bool sockepoller::get_so_sndtimeo(struct timeval *timeout)
{
	socklen_t len = sizeof(struct timeval);
	if (getsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, timeout, &len) == -1) {
		perror(DBG_PREFIX"getting SO_SNDTIMEO failed");
		return false;
	}

	if (len != sizeof(struct timeval)) {
		std::cerr << DBG_PREFIX"getting SO_SNDTIMEO failed, wrong length returned" << std::endl;
		return false;
	}

	return true;
}

bool sockepoller::set_so_tcp_nodelay(bool enabled)
{
	int nodelay = enabled;

	if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof nodelay) == -1) {
		perror(DBG_PREFIX"setting TCP_NODELAY failed");
		return false;
	}

	return true;
}

bool sockepoller::get_so_tcp_nodelay(bool *enabled)
{
	int nodelay;
	socklen_t len = sizeof nodelay;

	if (getsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &nodelay, &len) == -1) {
		perror(DBG_PREFIX"getting TCP_NODELAY failed");
		return false;
	}

	if (len != sizeof nodelay) {
		std::cerr << DBG_PREFIX"getting TCP_NODELAY failed, wrong length returned" << std::endl;
		return false;
	}

	*enabled = nodelay;

	return true;
}

bool sockepoller::set_so_tcp_cork(bool enabled)
{
	int cork = enabled;

	if (setsockopt(fd, IPPROTO_TCP, TCP_CORK, &cork, sizeof cork) == -1) {
		perror(DBG_PREFIX"setting TCP_CORK failed");
		return false;
	}

	return true;
}

bool sockepoller::get_so_tcp_cork(bool *enabled)
{
	int cork;
	socklen_t len = sizeof cork;

	if (getsockopt(fd, IPPROTO_TCP, TCP_CORK, &cork, &len) == -1) {
		perror(DBG_PREFIX"getting TCP_CORK failed");
		return false;
	}

	if (len != sizeof cork) {
		std::cerr << DBG_PREFIX"getting TCP_CORK failed, wrong length returned" << std::endl;
		return false;
	}

	*enabled = cork;

	return true;
}

bool sockepoller::set_so_tcp_keepidle(int value)
{
	if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &value, sizeof value) == -1) {
		perror(DBG_PREFIX"setting TCP_KEEPIDLE failed");
		return false;
	}

	return true;
}

bool sockepoller::get_so_tcp_keepidle(int *value)
{
	socklen_t len = sizeof(int);

	if (getsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, value, &len) == -1) {
		perror(DBG_PREFIX"getting TCP_KEEPIDLE failed");
		return false;
	}

	if (len != sizeof(int)) {
		std::cerr << DBG_PREFIX"getting TCP_KEEPIDLE failed, wrong length returned" << std::endl;
		return false;
	}

	return true;
}

bool sockepoller::set_so_tcp_keepintvl(int value)
{
	if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &value, sizeof value) == -1) {
		perror(DBG_PREFIX"setting TCP_KEEPINTVL failed");
		return false;
	}

	return true;
}

bool sockepoller::get_so_tcp_keepintvl(int *value)
{
	socklen_t len = sizeof(int);

	if (getsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, value, &len) == -1) {
		perror(DBG_PREFIX"getting TCP_KEEPINTVL failed");
		return false;
	}

	if (len != sizeof(int)) {
		std::cerr << DBG_PREFIX"getting TCP_KEEPINTVL failed, wrong length returned" << std::endl;
		return false;
	}

	return true;
}

bool sockepoller::set_so_tcp_keepcnt(int value)
{
	if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &value, sizeof value) == -1) {
		perror(DBG_PREFIX"setting TCP_KEEPCNT failed");
		return false;
	}

	return true;
}

bool sockepoller::get_so_tcp_keepcnt(int *value)
{
	socklen_t len = sizeof(int);

	if (getsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, value, &len) == -1) {
		perror(DBG_PREFIX"getting TCP_KEEPCNT failed");
		return false;
	}

	if (len != sizeof(int)) {
		std::cerr << DBG_PREFIX"getting TCP_KEEPCNT failed, wrong length returned" << std::endl;
		return false;
	}

	return true;
}

bool sockepoller::set_so_tcp_maxseg(int value)
{
	if (setsockopt(fd, IPPROTO_TCP, TCP_MAXSEG, &value, sizeof value) == -1) {
		perror(DBG_PREFIX"setting TCP_MAXSEG failed");
		return false;
	}

	return true;
}

bool sockepoller::get_so_tcp_maxseg(int *value)
{
	socklen_t len = sizeof(int);

	if (getsockopt(fd, IPPROTO_TCP, TCP_MAXSEG, value, &len) == -1) {
		perror(DBG_PREFIX"getting TCP_MAXSEG failed");
		return false;
	}

	if (len != sizeof(int)) {
		std::cerr << DBG_PREFIX"getting TCP_MAXSEG failed, wrong length returned" << std::endl;
		return false;
	}

	return true;
}

bool sockepoller::set_so_tcp_syncnt(int value)
{
	if (setsockopt(fd, IPPROTO_TCP, TCP_SYNCNT, &value, sizeof value) == -1) {
		perror(DBG_PREFIX"setting TCP_SYNCNT failed");
		return false;
	}

	return true;
}

bool sockepoller::get_so_tcp_syncnt(int *value)
{
	socklen_t len = sizeof(int);

	if (getsockopt(fd, IPPROTO_TCP, TCP_SYNCNT, value, &len) == -1) {
		perror(DBG_PREFIX"getting TCP_SYNCNT failed");
		return false;
	}

	if (len != sizeof(int)) {
		std::cerr << DBG_PREFIX"getting TCP_SYNCNT failed, wrong length returned" << std::endl;
		return false;
	}

	return true;
}

bool sockepoller::get_so_tcp_info(struct tcp_info *tcp_info)
{
	socklen_t len = sizeof(struct tcp_info);

	if (getsockopt(fd, SOL_TCP, TCP_INFO, tcp_info, &len) == -1) {
		perror(DBG_PREFIX"getting TCP_INFO failed");
		return false;
	}

	if (len != sizeof(struct tcp_info)) {
		std::cerr << DBG_PREFIX"getting TCP_INFO failed, wrong length returned" << std::endl;
		return false;
	}

	return true;
}

bool sockepoller::fill_addr_inet(const std::string &ip, unsigned short port, struct sockaddr_storage *addr, socklen_t *addr_len)
{
	int domain;

	if (!get_so_domain(&domain))
		return false;

	if (domain == AF_INET) {
		if (!fill_addr_inet4(ip, port, (struct sockaddr_in *) addr))
			return false;
		*addr_len = sizeof(struct sockaddr_in);

	} else if (domain == AF_INET6) {
		if (!fill_addr_inet6(ip, port, (struct sockaddr_in6 *) addr))
			return false;
		*addr_len = sizeof(struct sockaddr_in6);

	} else {
		std::cerr << DBG_PREFIX"unsupported socket domain" << std::endl;
		return false;
	}

	return true;
}

int sockepoller::epoll_in()
{
	int ret = recv(fd, LINBUFF_WR_PTR(&rxbuff), linbuff_towr(&rxbuff), rx_flags);

	if (ret < -1) {
		perror(DBG_PREFIX"reading from file descriptor failed (unexpected retvalue)");
		return rx(-1);

	} else if (ret == -1) {
		perror(DBG_PREFIX"reading from file descriptor failed");
		return rx(-1);

	} else if (ret == 0) {
		return rx(0);

	} else {
		linbuff_forward(&rxbuff, ret);
		return rx(ret);
	}
}

int sockepoller::epoll_out()
{
	int ret = send(fd, LINBUFF_RD_PTR(&txbuff), linbuff_tord(&txbuff), tx_flags);

	if (ret < -1) {
		perror(DBG_PREFIX"writing to file descriptor failed (unexpected retvalue)");
		return tx(-1);

	} else if (ret == -1) {
		perror(DBG_PREFIX"writing to file descriptor failed");
		return tx(-1);

	} else if (ret == 0) {
		return tx(0);

	} else {
		linbuff_skip(&txbuff, ret);
		return tx(ret);
	}
}

void sockepoller::print_tcp_info(const struct tcp_info *tcp_info, std::ostream &out)
{
	out << "state          : " << static_cast<unsigned int>(tcp_info->tcpi_state)
	                           << " [" << tcp_state_2_str(tcp_info->tcpi_state)
	                           << "]"  << std::endl;

	out << "ca_state       : " << static_cast<unsigned int>(tcp_info->tcpi_ca_state)       << std::endl;
	out << "retransmits    : " << static_cast<unsigned int>(tcp_info->tcpi_retransmits)    << std::endl;
	out << "probes         : " << static_cast<unsigned int>(tcp_info->tcpi_probes)         << std::endl;
	out << "backoff        : " << static_cast<unsigned int>(tcp_info->tcpi_backoff)        << std::endl;
	out << "options        : " << static_cast<unsigned int>(tcp_info->tcpi_options)        << std::endl;
	out << "snd_wscale     : " << static_cast<unsigned int>(tcp_info->tcpi_snd_wscale)     << std::endl;
	out << "rcv_wscale     : " << static_cast<unsigned int>(tcp_info->tcpi_rcv_wscale)     << std::endl;

	out << "rto            : " << static_cast<unsigned int>(tcp_info->tcpi_rto)            << std::endl;
	out << "ato            : " << static_cast<unsigned int>(tcp_info->tcpi_ato)            << std::endl;
	out << "snd_mss        : " << static_cast<unsigned int>(tcp_info->tcpi_snd_mss)        << std::endl;
	out << "rcv_mss        : " << static_cast<unsigned int>(tcp_info->tcpi_rcv_mss)        << std::endl;

	out << "unacked        : " << static_cast<unsigned int>(tcp_info->tcpi_unacked)        << std::endl;
	out << "sacked         : " << static_cast<unsigned int>(tcp_info->tcpi_sacked)         << std::endl;
	out << "lost           : " << static_cast<unsigned int>(tcp_info->tcpi_lost)           << std::endl;
	out << "retrans        : " << static_cast<unsigned int>(tcp_info->tcpi_retrans)        << std::endl;
	out << "fackets        : " << static_cast<unsigned int>(tcp_info->tcpi_fackets)        << std::endl;

	// times
	out << "last_data_sent : " << static_cast<unsigned int>(tcp_info->tcpi_last_data_sent) << std::endl;
	out << "last_ack_sent  : " << static_cast<unsigned int>(tcp_info->tcpi_last_ack_sent)  << std::endl;
	out << "last_data_recv : " << static_cast<unsigned int>(tcp_info->tcpi_last_data_recv) << std::endl;
	out << "lsast_ack_recv : " << static_cast<unsigned int>(tcp_info->tcpi_last_ack_recv)  << std::endl;

	// metrics
	out << "pmtu           : " << static_cast<unsigned int>(tcp_info->tcpi_pmtu)           << std::endl;
	out << "rcv_ssthresh   : " << static_cast<unsigned int>(tcp_info->tcpi_rcv_ssthresh)   << std::endl;
	out << "rtt            : " << static_cast<unsigned int>(tcp_info->tcpi_rtt)            << std::endl;
	out << "rttvar         : " << static_cast<unsigned int>(tcp_info->tcpi_rttvar)         << std::endl;
	out << "snd_ssthresh   : " << static_cast<unsigned int>(tcp_info->tcpi_snd_ssthresh)   << std::endl;
	out << "snd_cwnd       : " << static_cast<unsigned int>(tcp_info->tcpi_snd_cwnd)       << std::endl;
	out << "advmss         : " << static_cast<unsigned int>(tcp_info->tcpi_advmss)         << std::endl;
	out << "reordering     : " << static_cast<unsigned int>(tcp_info->tcpi_reordering)     << std::endl;

	out << "rcv_rtt        : " << static_cast<unsigned int>(tcp_info->tcpi_rcv_rtt)        << std::endl;
	out << "rcv_space      : " << static_cast<unsigned int>(tcp_info->tcpi_rcv_space)      << std::endl;

	out << "total_retrans  : " << static_cast<unsigned int>(tcp_info->tcpi_total_retrans)  << std::endl;
}

std::string sockepoller::tcp_state_2_str(int state)
{
	switch (state)
	{
		case TCP_ESTABLISHED : return "TCP_ESTABLISHED";
		case TCP_SYN_SENT    : return "TCP_SYN_SENT";
		case TCP_SYN_RECV    : return "TCP_SYN_RECV";
		case TCP_FIN_WAIT1   : return "TCP_FIN_WAIT1";
		case TCP_FIN_WAIT2   : return "TCP_FIN_WAIT2";
		case TCP_TIME_WAIT   : return "TCP_TIME_WAIT";
		case TCP_CLOSE       : return "TCP_CLOSE";
		case TCP_CLOSE_WAIT  : return "TCP_CLOSE_WAIT";
		case TCP_LAST_ACK    : return "TCP_LAST_ACK";
		case TCP_LISTEN      : return "TCP_LISTEN";
		case TCP_CLOSING     : return "TCP_CLOSING";
		default              : return "<unknown>";
	}
}

std::string sockepoller::so_domain_2_str(int domain)
{
	switch (domain)
	{
		case AF_UNIX      : return "AF_UNIX";
		case AF_INET      : return "AF_INET";
		case AF_INET6     : return "AF_INET6";
		case AF_IPX       : return "AF_IPX";
		case AF_NETLINK   : return "AF_NETLINK";
		case AF_X25       : return "AF_X25";
		case AF_AX25      : return "AF_AX25";
		case AF_ATMPVC    : return "AF_ATMPVC";
		case AF_APPLETALK : return "AF_APPLETALK";
		case AF_PACKET    : return "AF_PACKET";
		default           : return "<unknown>";
	}
}

std::string sockepoller::so_type_2_str(int type)
{
	switch (type)
	{
		case SOCK_STREAM    : return "SOCK_STREAM";
		case SOCK_DGRAM     : return "SOCK_DGRAM";
		case SOCK_SEQPACKET : return "SOCK_SEQPACKET";
		case SOCK_RAW       : return "SOCK_RAW";
		case SOCK_RDM       : return "SOCK_RDM";
		case SOCK_PACKET    : return "SOCK_PACKET";
		default             : return "<unknown>";
	}
}

bool sockepoller::ip2family(const std::string &ip, int *ai_family)
{
	struct addrinfo hint = {};
	struct addrinfo *res = NULL;

	hint.ai_family = PF_UNSPEC;
	hint.ai_flags = AI_NUMERICHOST;

	if (!getaddrinfo(ip.c_str(), NULL, &hint, &res)) {
		*ai_family = res->ai_family;
		return true;
	} else
		return false;
}

bool sockepoller::fill_addr_inet4(const std::string &ip, unsigned short port, struct sockaddr_in *addr)
{
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);
	if (ip.empty())
		addr->sin_addr.s_addr = htonl(INADDR_ANY);
	else {
		int ret = inet_pton(AF_INET, ip.c_str(), &(addr->sin_addr));
		if (ret == 0) {
			std::cerr << DBG_PREFIX"converting IPv4 address failed, invalid network address" << std::endl;
			return false;
		 } else if (ret == -1) {
			perror(DBG_PREFIX"converting IPv4 address failed");
			return false;
		 }
	}

	return true;
}

bool sockepoller::fill_addr_inet6(const std::string &ip, unsigned short port, struct sockaddr_in6 *addr)
{
	addr->sin6_family = AF_INET6;
	addr->sin6_port = htons(port);
	if (ip.empty())
		addr->sin6_addr = in6addr_any;
	else {
		int ret = inet_pton(AF_INET6, ip.c_str(), &(addr->sin6_addr));
		if (ret == 0) {
			std::cerr << DBG_PREFIX"converting IPv6 address failed, invalid network address" << std::endl;
			return false;
		 } else if (ret == -1) {
			perror(DBG_PREFIX"converting IPv6 address failed");
			return false;
		 }
	}

	return true;
}

bool sockepoller::ip2str_inet4(const struct in_addr *addr, std::string &str)
{
	char ip[INET_ADDRSTRLEN];
	if(!inet_ntop(AF_INET, addr, ip, INET_ADDRSTRLEN))
		return false;
	str = ip;
	return true;
}

bool sockepoller::ip2str_inet6(const struct in6_addr *addr, std::string &str)
{
	char ip6[INET6_ADDRSTRLEN];
	if(!inet_ntop(AF_INET6, addr, ip6, INET6_ADDRSTRLEN))
		return false;
	str = ip6;
	return true;
}

bool sockepoller::str2ip_inet4(struct in_addr *addr, const std::string &str)
{
	return inet_pton(AF_INET, str.c_str(), addr) == 1 ? true : false;
}

bool sockepoller::str2ip_inet6(struct in6_addr *addr, const std::string &str)
{
	return inet_pton(AF_INET6, str.c_str(), addr) == 1 ? true : false;
}

