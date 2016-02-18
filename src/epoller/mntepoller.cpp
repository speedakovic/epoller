#include <epoller/mntepoller.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <sstream>

#define DBG_PREFIX "mntepoller: "

bool mntepoller::open(const std::string &pathname)
{
	if (!fdepoller::open(pathname,
	                     O_RDONLY,
	                     sysconf(_SC_PAGESIZE), /* rxsize */
	                     0,                     /* txsize */
	                     true,                  /* rxen   */
	                     false,                 /* txen   */
	                     true)) {               /* en     */
		std::cerr << DBG_PREFIX"opening " << pathname << " failed" << std::endl;
		return false;
	}

	return true;
}

int mntepoller::rx(int len)
{
	if (len < 0) {
		return -1;
	} else if (len == 0) {
		disable_rx();
		return done();
	} else {
		if (!linbuff_towr(&rxbuff) && !linbuff_realloc(&rxbuff, 2 * rxbuff.size)) {
			std::cerr << DBG_PREFIX"buffer reallocation failed" << std::endl;
			return -1;
		}
		return 0;
	}
}

int mntepoller::err()
{
	lseek(fd, 0, SEEK_SET);
	enable_rx();
	return 0;
}

int mntepoller::done()
{
	std::list<mntentry> entries;
	std::string sline;
	std::stringstream sslines(std::string((char *)LINBUFF_RD_PTR(&rxbuff), linbuff_tord(&rxbuff)));

	while (std::getline(sslines, sline)) {
		mntentry entry;
		std::string sitem;
		std::stringstream ssline(sline);

		ssline >> entry.fsname;
		ssline >> entry.dir;
		ssline >> entry.type;
		ssline >> entry.opts;
		ssline >> entry.freq;
		ssline >> entry.passno;

		entries.push_back(entry);
	}

	change(entries);
	linbuff_clear(&rxbuff);

	return 0;
}

int mntepoller::change(const std::list<mntentry> &entries)
{
	if (_change)
		return _change(this, entries);
	else
		return 0;
}

void mntepoller::print_mntentries(const std::list<mntentry> &entries, std::ostream &out)
{
	for (std::list<mntentry>::const_iterator it = entries.begin(); it != entries.end(); ++it)
		out << (*it).fsname << "|"
		    << (*it).dir    << "|"
		    << (*it).type   << "|"
		    << (*it).opts   << "|"
		    << (*it).freq   << "|"
		    << (*it).passno << std::endl;
}

