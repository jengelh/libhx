======================================
ux-file - Unix compatibility functions
======================================

Date authored: 2006-02-25

Description
===========

libHX provides some dummy Unix functions for platforms where they are not
available. They mostly return `-ENOSYS`.

Synopsis
========

.. code-block:: c

	#include <libHX.h>

	int chown(const char *PATH, long UID, long GID);
	int fchmod(int FD, long PERM);
	int fchown(int FD, long UID, long GID);
	int lchown(const char *PATH, long UID, long GID);
	int lstat(const char *PATH, struct stat *SB);
	int mkfifo(const char *PATH, long MODE);
	int mknod(const char *PATH, long MODE, long DEV);
	int readlink(const char *PATH, char *DEST, size_t LEN);
	int symlink(const char *SRC, const char *DEST);

``lstat()``
	Maps to ``stat()`` under Win32.
