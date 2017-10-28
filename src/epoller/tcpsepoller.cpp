#include <epoller/tcpsepoller.h>
#include <fcntl.h>

#define DBG_PREFIX "tcpsepoller: "

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
	return rcvr ? dynamic_cast<receiver *>(rcvr)->acc(*this, fd, addr, addrlen) : (_acc ? _acc(*this, fd, addr, addrlen) : 0);
}

int tcpsepoller::epoll_in()
{
	struct sockaddr addr;
	socklen_t addrlen = sizeof addr;

	return acc(accept(&addr, &addrlen), &addr, &addrlen);
}

