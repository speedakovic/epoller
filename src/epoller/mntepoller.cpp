#include <epoller/mntepoller.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <fstream>
#include <sstream>

#define DBG_PREFIX "mntepoller: "

static inline mntentry parse_entry(const std::string sline)
{
	mntentry entry;
	std::stringstream ssline(sline);

	ssline >> entry.fsname;
	ssline >> entry.dir;
	ssline >> entry.type;
	ssline >> entry.opts;
	ssline >> entry.freq;
	ssline >> entry.passno;

	return entry;
}

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
	std::string sline;
	std::list<mntentry> entries;
	std::stringstream sslines(std::string((char *)LINBUFF_RD_PTR(&rxbuff), linbuff_tord(&rxbuff)));

	while (std::getline(sslines, sline))
		entries.push_back(parse_entry(sline));

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

bool mntepoller::read(std::list<mntentry> &entries, const std::string &pathname)
{
	entries.clear();

	std::ifstream ifs(pathname.c_str());
	if (!ifs.is_open()) {
		std::cerr << DBG_PREFIX"opening " << pathname << " failed" << std::endl;
		return false;
	}

	std::string sline;
	while (std::getline(ifs, sline))
		entries.push_back(parse_entry(sline));

	return true;
}

std::list<mntentry> mntepoller::find_by_fsname(const std::list<mntentry> &entries, const std::string &fsname)
{
	std::list<mntentry> result;
	for (std::list<mntentry>::const_iterator it = entries.begin(); it != entries.end(); ++it)
		if ((*it).fsname == fsname)
			result.push_back(*it);
	return result;
}

std::list<mntentry> mntepoller::find_by_dir(const std::list<mntentry> &entries, const std::string &dir)
{
	std::list<mntentry> result;
	for (std::list<mntentry>::const_iterator it = entries.begin(); it != entries.end(); ++it)
		if ((*it).dir == dir)
			result.push_back(*it);
	return result;
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

