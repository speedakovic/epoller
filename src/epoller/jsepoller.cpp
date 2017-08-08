#include <epoller/jsepoller.h>
#include <fcntl.h>
#include <iostream>

#define DBG_PREFIX "jsepoller: "

bool jsepoller::open(const std::string &pathname, size_t evbuff)
{
	if (!fdepoller::open(pathname,
	                     O_RDONLY,
	                     evbuff * sizeof(struct js_event), /* rxsize */
	                     0,                                /* txsize */
	                     true,                             /* rxen   */
	                     false,                            /* txen   */
	                     true)) {                          /* en     */
		std::cerr << DBG_PREFIX"opening " << pathname << " failed" << std::endl;
		return false;
	}

	return true;
}

size_t jsepoller::get_axes()
{
	char num;
	ioctl(fd, JSIOCGAXES, &num);
	return num;
}

size_t jsepoller::get_buttons()
{
	char num;
	ioctl(fd, JSIOCGBUTTONS, &num);
	return num;
}

int jsepoller::get_version()
{
	int ver;
	ioctl(fd, JSIOCGVERSION, &ver);
	return ver;
}

std::string jsepoller::get_name()
{
	char name[128];
	return ioctl(fd, JSIOCGNAME(sizeof(name)), name) < 0 ? "<unknown>" : name;
}

int jsepoller::get_corr(struct js_corr *corr)
{
	return ioctl(fd, JSIOCGCORR, corr);
}

int jsepoller::set_corr(const struct js_corr *corr)
{
	return ioctl(fd, JSIOCSCORR, corr);
}

int jsepoller::rx(int len)
{
	int ret;

	if (len < 0) {
		return -1;

	} else if (len == 0) {
		return -1;

	} else {

		while (linbuff_tord(&rxbuff) >= sizeof(struct js_event)) {

			struct js_event *event = (struct js_event*)LINBUFF_RD_PTR(&rxbuff);

			if (ret = jshandler(event))
				return ret;

			linbuff_skip(&rxbuff, sizeof(struct js_event));
		}

		linbuff_compact(&rxbuff);
		return 0;
	}
}

int jsepoller::jshandler(struct js_event *event)
{
	return _jshandler ? _jshandler(this, event) : 0;
}

