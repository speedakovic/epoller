#include <epoller/tcpcepoller.h>
#include <fcntl.h>
#include <iostream>

#define DBG_PREFIX "tcpcepoller: "

bool tcpcepoller::socket(int domain, size_t rxsize, size_t txsize)
{
	if (!sockepoller::socket(domain, SOCK_STREAM | O_NONBLOCK, 0, rxsize, txsize, false, false, false))
		return false;

	return true;
}

void tcpcepoller::close()
{
	sockepoller::close();
	connecting = false;
}

bool tcpcepoller::connect(const std::string &ip, unsigned short port)
{
	if (!sockepoller::enable(false, true, false))
		return false;

	if (!sockepoller::connect(ip, port)) {
		sockepoller::disable();
		return false;
	}

	connecting = true;

	return true;
}

int tcpcepoller::cdone(bool connected)
{
	return _cdone ? _cdone(this, connected) : 0;
}

int tcpcepoller::epoll_out()
{
	if (connecting) {

		connecting = false;

		int ret = send(fd, 0, 0, 0);

		if (ret == 0) {
			return cdone(true);

		} else {
			perror(DBG_PREFIX"connecting failed");
			return cdone(false);
		}

	} else
		return sockepoller::epoll_out();
}

int tcpcepoller::epoll_hup()
{
	connecting = false;
	sockepoller::epoll_hup();
}

int tcpcepoller::epoll_err()
{
	connecting = false;
	sockepoller::epoll_err();
}

int tcpcepoller::epoll_unknown(int events)
{
	connecting = false;
	sockepoller::epoll_unknown(events);
}

