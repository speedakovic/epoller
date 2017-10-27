/// @file   epoller/mntepoller.h
/// @author speedak
/// @brief  Mount epoller.

#ifndef MNTEPOLLER_H
#define MNTEPOLLER_H

#include <paths.h>
#include <epoller/fdepoller.h>
#include <list>
#include <iostream>

/// @brief Mount file entry. The members correspond with those
///        ones in @c mntent structure defined in @c mntent.h.
struct mntentry
{
	std::string fsname;   ///< name of mounted filesystem
	std::string dir;      ///< filesystem path prefix
	std::string type;     ///< mount type (see mntent.h)
	std::string opts;     ///< mount options (see mntent.h)
	int         freq;     ///< dump frequency in days
	int         passno;   ///< pass number on parallel fsck
};

/// @brief Mount epoller. It waits for any change of mount file (typically @c /etc/mtab)
///        and when the change occurs appropriate callback is called.
struct mntepoller : fdepoller
{
	/// @brief Event receiver interface.
	struct receiver : fdepoller::receiver
	{
		/// @brief Destructor.
		virtual ~receiver() {}

		/// @brief Called whenever mount entries have changed.
		/// @param sender event sender
		/// @param entries mount entries
		/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
		virtual int change(mntepoller &sender, const std::list<mntentry> &entries) = 0;
	};

	/// @brief Called whenever mount entries have changed.
	/// @param sender event sender
	/// @param entries mount entries
	/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
	int (*_change) (mntepoller &sender, const std::list<mntentry> &entries);

	/// @brief Constructor.
	/// @param epoller parent epoller
	mntepoller(struct epoller *epoller) : fdepoller(epoller), _change(0) {
		rx_auto_enable  = false;
		rx_auto_disable = false;
		tx_auto_enable  = false;
		tx_auto_disable = false;
	}

	/// @brief Default constructor.
	mntepoller() : mntepoller(0) {}

	/// @brief Destructor.
	virtual ~mntepoller() {}

	/// @brief Opens mount file.
	/// @param pathname mount file
	/// @return @c true if opening was successful, otherwise @c false
	virtual bool open(const std::string &pathname = _PATH_MOUNTED);

	/// @brief This handler is called whenever mount file is available to be read.
	///        Zero len means end of file and reception is disabled.
	/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
	virtual int rx(int len);

	/// @brief This handler is called whenever mount file has changed.
	///        File is seeked to its beginning and reception is enabled.
	/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
	virtual int err();

	/// @brief This handler is called whenever whole mount file has been read.
	///        This handler processes and frees rx buffer.
	/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
	virtual int done();

	/// @brief This handler is called whenever mount entries have changed.
	///
	/// Default implementation calls receiver::change method of #rcvr if not null,
	/// otherwise calls #_change if not null,
	/// otherwise returns 0.
	///
	/// @param entries mount entries
	/// @return zero for loop continuation, positive for normal loop exit, negative for loop exit with error
	virtual int change(const std::list<mntentry> &entries);

	/// @brief Reads mount file.
	/// @param entries returned mount entries
	/// @param pathname mount file
	/// @return @c true if reading was successful, otherwise @c false
	static bool read(std::list<mntentry> &entries, const std::string &pathname = _PATH_MOUNTED);

	/// @brief Finds mount entries by fsname.
	/// @param entries entries to be searched
	/// @param fsname entries to find
	/// @return found entries
	static std::list<mntentry> find_by_fsname(const std::list<mntentry> &entries, const std::string &fsname);

	/// @brief Finds mount entries by dir.
	/// @param entries entries to be searched
	/// @param dir entries to find
	/// @return found entries
	static std::list<mntentry> find_by_dir(const std::list<mntentry> &entries, const std::string &dir);

	/// @brief Prints mount entries.
	/// @param entries mount entries
	/// @param out output stream
	static void print_mntentries(const std::list<mntentry> &entries, std::ostream &out = std::cout);
};

#endif // MNTEPOLLER_H

