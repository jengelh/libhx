===========================
File and directory handling
===========================


Directory traversal
===================

libHX provides a minimal readdir-style wrapper for cross-platform directory
traversal. This is needed because the Win32 platforms does not have readdir,
and there is some housekeeping to do on Unixish platforms, since the dirent
structure needs allocation of a path-specific size.

.. code-block:: c

	#include <libHX/io.h>

	struct HXdir *HXdir_open(const char *directory);
	const char *HXdir_read(struct HXdir *handle);
	void HXdir_close(struct HXdir *handle);

``HXdir_open`` returns a pointer to its private data area, or ``NULL`` upon
failure, in which case ``errno`` is preserved from the underlying system calls.
``HXdir_read`` causes the next entry from the directory to be fetched. The
pointer returned by ``HXdir_read`` must not be freed, and the data is
overwritten in subsequent calls to the same handle. If you want to keep it
around, you will have to duplicate it yourself. ``HXdir_close`` will close the
directory and free the private data it held.


Example
-------

.. code-block:: c

	#include <errno.h>
	#include <stdio.h>
	#include <libHX/io.h>

	struct HXdir *dh;
	if ((dh = HXdir_open(".")) == NULL) {
		fprintf(stderr, "Could not open directory: %s\n", strerror(errno));
		return;
	}
	while ((dentry = HXdir_read(dh)) != NULL)
		printf("%s\n", dentry);
	HXdir_close(dh);

This sample will open the current directory, and print out all entries as it
iterates over them.


Operation on directory entries
==============================

.. code-block:: c

	#include <libHX/io.h>

	int HX_readlink(hxmc_t **buf, const char *path);
	int HX_realpath(hxmc_t **buf, const char *path, unsigned int flags);

``HX_readlink`` calls through to readlink to read the target of a symbolic
link, and stores the result in the memory container referenced by ``*buf``
(similar to ``HX_getl`` semantics). If ``*buf`` is ``NULL``, a new container
will be allocated and a pointer to it stored in ``*buf``. The container's
content is naturally zero-terminated automatically. The return value of the
function will be the length of the link target, or negative to indicate the
system error value.

``HX_realpath`` will normalize the given path by transforming various path
components into alternate descriptions. The flags parameter controls its
actions:

``HX_REALPATH_DEFAULT``
	A mnemonic for a set of standard flags: ``HX_REALPATH_SELF |
	HX_REALPATH_PARENT``. Note that ``HX_REALPATH_ABSOLUTE``, which would
	also be required to get libc's ``realpath``(3) behavior, is not
	included in the set.

``HX_REALPATH_ABSOLUTE``
	Requests that the output path shall be absolute. In the absence of this
	flag, an absolute output path will only be produced if the input path
	is also absolute.

``HX_REALPATH_SELF``
	Request resolution of `.` path components.

``HX_REALPATH_PARENT`
	Request resolution of `..` path components.

The result is stored in a memory container whose pointer is returned through
``*buf``. The return value of the function will be negative to indicate a
possible system error, or be positive non-zero for success.


Operations on directories
=========================

.. code-block:: c

	#include <libHX/io.h>

	int HX_mkdir(const char *path, unsigned int mode);
	int HX_rrmdir(const char *path);

``HX_mkdir`` will create the directory given by path and all its parents that
do not exist yet using the given mode. It is equivalent to the ``mkdir -p``
shell command. It will return >0 for success, or ``-errno`` on error.

``HX_rrmdir`` also maps to an operation commonly done on the shell, ``rm -Rf``,
deleting the directory given by path, including all files within it and its
subdirectories. Errors during deletion are ignored, but if there was any, the
errno value of the first one is returned negated.


Operations on files
===================

.. code-block:: c

	#include <libHX/io.h>

	#define HXF_KEEP ...
	#define HXF_UID ...
	#define HXF_GID ...

	int HX_copy_file(const char *src, const char *dest, unsigned int flags, ...);
	int HX_copy_dir(const char *src, const char *dest, unsigned int flags, ...);
	char *HX_slurp_fd(int fd, size_t *outsize);
	char *HX_slurp_file(const char *file, size_t *outsize);

``HX_copy_file``
	Copies one named file to a new location. Possible ``flags`` are
	``HXF_KEEP``, ``HXF_UID`` and ``HXF_GID``. Error checking by
	``HX_copy_file`` is flakey. ``HX_copy_file`` will return >0 on success,
	or ``-errno`` on failure. Errors can arise from the use of the syscalls
	``open``, ``read`` and ``write``. The return value of ``fchmod``, which
	is used to set the UID and GID, is actually ignored, which means
	verifying that the owner has been set cannot be detected with
	``HX_copy_file`` alone (historic negligience?).

``HXF_KEEP``
	Do not overwrite existing files.

``HXF_UID``
	Change the new file's owner to the UID given in the varargs section
	(...). ``HXF_UID`` is processed before ``HXF_GID``.

``HXF_GID``
	Change the new file's group owner to the GID given in the varargs
	section. This is processed after ``HXF_UID``.

``HX_copy_dir``
	Copies one named directory to a new location, recursively.
	(Uses ``HX_copy_file`` and ``HX_copy_dir``.) Error checking by
	``HX_copy_dir`` is flakey.

``HX_slurp_fd``
	Reads all remaining bytes from the given filedescriptor ``fd`` and
	returns a pointer to a newly-allocated content buffer. If ``outsize``
	is not ``NULL``, the size of the buffer will be written to it. The
	buffer is always terminated by a gratuitious NUL (not counted in
	``outsize``). Once no longer needed, the buffer should be released with
	``free``.

``HX_slurp_file``
	Reads all bytes from the given filename and returns a pointer to the
	content buffer. Inherits all the characteristics from ``HX_slurp_fd``.


Filedescriptor helpers
======================

.. code-block:: c

	#include <libHX/io.h>

	ssize_t HXio_fullread(int fd, void *buf, size_t size, unsigned int flags);
	ssize_t HXio_fullwrite(int fd, const void *buf, size_t size, unsigned int flags);

Since plain ``read``(2) and ``write``(2) may process only part of the buffer —
even more likely so with sockets —, libHX provides two functions that calls
these in a loop to retry said operations until the full amount has been
processed. Since read and write can also be used with socket file descriptors,
so can these.
