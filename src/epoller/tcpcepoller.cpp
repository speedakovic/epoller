#include <epoller/tcpcepoller.h>
#include <fcntl.h>
#include <iostream>

#define DBG_PREFIX "tcpcepoller: "

int tcpcepoller::receiver::con(tcpcepoller &sender, bool connected)
{
	std::cerr << DBG_PREFIX"unhandled event: con" << std::endl;
	return -1;
}

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

int tcpcepoller::con(bool connected)
{
	if (rcvr)
		return dynamic_cast<receiver *>(rcvr)->con(*this, connected);
	else if (_con)
		return _con(*this, connected);
	else {
		std::cerr << DBG_PREFIX"unhandled event: con" << std::endl;
		return -1;
	}
}

int tcpcepoller::epoll_out()
{
	if (connecting) {
		connecting = false;
		return con(send(fd, 0, 0, 0) ? false : true);

	} else
		return sockepoller::epoll_out();
}

int tcpcepoller::epoll_hup()
{
	connecting = false;
	return sockepoller::epoll_hup();
}

int tcpcepoller::epoll_err()
{
	connecting = false;
	return sockepoller::epoll_err();
}

int tcpcepoller::epoll_unknown(int events)
{
	connecting = false;
	return sockepoller::epoll_unknown(events);
}

