#include <epoller/tcpsepoller.h>
#include <fcntl.h>

#define DBG_PREFIX "tcpsepoller: "

int tcpsepoller::receiver::acc(tcpsepoller &sender, int fd, const struct sockaddr *addr, const socklen_t *addrlen)
{
	std::cerr << DBG_PREFIX"unhandled event: acc" << std::endl;
	return -1;
}

bool tcpsepoller::socket(int domain, const std::string &ip, unsigned short port, int backlog, bool reuseaddr)
{
	if (fd != -1)
		return true;

	if (!sockepoller::socket(domain, SOCK_STREAM | O_NONBLOCK, 0, 1, 0, true, false, true))
		return false;

	if (!set_so_reuseaddr(reuseaddr)) {
		close();
		return false;
	}

	if (!bind(ip, port)) {
		close();
		return false;
	}

	if (!listen(backlog)) {
		close();
		return false;
	}

	return true;
}

int tcpsepoller::acc(int fd, const struct sockaddr *addr, const socklen_t *addrlen)
{
	if (rcvr)
		return dynamic_cast<receiver *>(rcvr)->acc(*this, fd, addr, addrlen);
	else if (_acc)
		return _acc(*this, fd, addr, addrlen);
	else {
		std::cerr << DBG_PREFIX"unhandled event: acc" << std::endl;
		return -1;
	}
}

int tcpsepoller::epoll_in()
{
	struct sockaddr addr;
	socklen_t addrlen = sizeof addr;

	return acc(accept(&addr, &addrlen), &addr, &addrlen);
}

