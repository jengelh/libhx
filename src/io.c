/*
 *	File and directory handling
 *	Copyright Jan Engelhardt, 2002-2011
 *
 *	This file is part of libHX. libHX is free software; you can
 *	redistribute it and/or modify it under the terms of the GNU Lesser
 *	General Public License as published by the Free Software Foundation;
 *	either version 2.1 or (at your option) any later version.
 */
#define _GNU_SOURCE 1
#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif
#include <sys/stat.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined _WIN32
#	include <direct.h>
#	include <io.h>
#	include <windows.h>
#else
#	include <dirent.h>
#	include <unistd.h>
#endif
#if __linux__
#	include <sys/sendfile.h>
#endif
#include <libHX/ctype_helper.h>
#include <libHX/defs.h>
#include <libHX/io.h>
#include <libHX/misc.h>
#include <libHX/string.h>
#include "internal.h"
#ifndef O_CLOEXEC
#	define O_CLOEXEC 0
#endif

struct HXdir {
#if defined _WIN32
	char *dname;
	HANDLE ptr;
	WIN32_FIND_DATA dentry;
	bool got_first;
#else
	DIR *ptr;
	struct dirent *dentry;
#endif
};

static int mkdir_gen(const char *d, unsigned int mode)
{
	struct stat sb;
#if defined(_WIN32)
	if (mkdir(d) == 0)
#else
	if (mkdir(d, mode) == 0) /* use umask() for permissions */
#endif
		return 1;
	if (errno != EEXIST)
		return -errno;
	if (stat(d, &sb) == 0) {
#if defined(_WIN32)
		if (sb.st_mode & S_IFDIR)
#else
		if (S_ISDIR(sb.st_mode))
#endif
			return 0;
	}
	return -EEXIST;
}

EXPORT_SYMBOL struct HXdir *HXdir_open(const char *s)
{
	struct HXdir *d;

#ifdef _WIN32
	if ((d = malloc(sizeof(*d))) == NULL)
		return NULL;
	if ((d->dname = malloc(strlen(s) + 3)) == NULL) {
		free(d);
		return NULL;
	}
	strcpy(d->dname, s);
	strcat(d->dname, "\\*");
	d->got_first = false;
#else
	/*
	 * On Linux-glibc, the struct dirent definition contains a wasteful
	 * and bug-concealing "char d_name[256]", while on Solaris, it is a
	 * proper "char d_name[]".
	 */
	size_t size = sizeof(*d);
	ssize_t name_max;
	DIR *tmp_dh = opendir(s);
	if (tmp_dh == NULL)
		return NULL;
	/*
	 * dirfd is POSIX.1-2008 and was present earlier in selected
	 * extensions. In case of !HAVE_DIRFD, use pathconf(s, _PC_NAME_MAX)
	 * and bite the race bullet.
	 */
	name_max = fpathconf(dirfd(tmp_dh), _PC_NAME_MAX);
	if (name_max > 0) {
		size -= sizeof(struct dirent) - offsetof(struct dirent, d_name);
		size += name_max + 1;
	} else {
#ifdef NAME_MAX
		size += NAME_MAX; /* "best effort" :-/ */
#elif defined(MAXNAMELEN)
		size += MAXNAMELEN;
#else
		fprintf(stderr, "libHX-warning: Cannot determine buffer size for readdir\n");
		closedir(tmp_dh);
		errno = EINVAL;
		return NULL;
#endif
	}
	if ((d = malloc(size)) == NULL) {
		closedir(tmp_dh);
		return NULL;
	}
	d->ptr = tmp_dh;
#endif
	return d;
}

EXPORT_SYMBOL const char *HXdir_read(struct HXdir *d)
{
	if (d == NULL)
		return NULL;

	errno = 0;
#if defined _WIN32
	if (!d->got_first) {
		d->got_first = true;
		if ((d->ptr = FindFirstFile(d->dname, &d->dentry)) == NULL)
			return NULL;
	} else if (!FindNextFile(d->ptr, &d->dentry)) {
		return NULL;
	}
	return d->dentry.cFileName;
#else
	d->dentry = readdir(d->ptr);
	if (d->dentry == NULL)
		return NULL;
	return d->dentry->d_name;
#endif
}

EXPORT_SYMBOL void HXdir_close(struct HXdir *d)
{
	if (d == NULL)
		return;
#if defined _WIN32
	FindClose(d->ptr);
	free(d->dname);
#else
	closedir(d->ptr);
#endif
	free(d);
}

EXPORT_SYMBOL int HX_copy_file(const char *src, const char *dest,
    unsigned int opts, ...)
{
	static const size_t bufsize = 0x10000;
	void *buf;
	unsigned int extra_flags = 0;
	int srcfd, dstfd;

	buf = malloc(bufsize);
	if (buf == nullptr)
		return -errno;
	srcfd = open(src, O_RDONLY | O_BINARY);
	if (srcfd < 0) {
		free(buf);
		return -errno;
	}
	if (opts & HXF_KEEP)
		extra_flags = O_EXCL;
	dstfd = open(dest, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC |
	        extra_flags, S_IRUGO | S_IWUGO);
	if (dstfd < 0) {
		int saved_errno = errno;
		free(buf);
		close(srcfd);
		return -(errno = saved_errno);
	}

	if (opts & (HXF_UID | HXF_GID)) {
		struct stat sb;
		long uid, gid;
		va_list argp;
		va_start(argp, opts);

		if (fstat(dstfd, &sb) < 0) {
			int saved_errno = errno;
			unlink(dest);
			close(dstfd);
			close(srcfd);
			free(buf);
			va_end(argp);
			return -(errno = saved_errno);
		}
		uid = sb.st_uid;
		gid = sb.st_gid;

		if (opts & HXF_UID) uid = va_arg(argp, long);
		if (opts & HXF_GID) gid = va_arg(argp, long);
		if (fchown(dstfd, uid, gid) < 0) {
			int saved_errno = errno;
			unlink(dest);
			close(dstfd);
			close(srcfd);
			free(buf);
			va_end(argp);
			return -(errno = saved_errno);
		}
		va_end(argp);
	}

	while (true) {
		ssize_t rdret = HX_sendfile(dstfd, srcfd, SIZE_MAX);
		if (rdret == 0)
			break;
		if (rdret < 0 && errno != EINTR) {
			int saved_errno = errno;
			close(srcfd);
			close(dstfd);
			free(buf);
			return -(errno = saved_errno);
		}
	}
	close(srcfd);
	close(dstfd);
	free(buf);
	return 1;
}

EXPORT_SYMBOL int HX_copy_dir(const char *src, const char *dest,
    unsigned int opts, ...)
{
	void *dt = HXdir_open(src);
	long uid = -1, gid = -1;
	const char *fn;

	if (dt == NULL)
		return 0;

	{
		va_list argp;
		va_start(argp, opts);
		if (opts & HXF_UID) uid = va_arg(argp, long);
		if (opts & HXF_GID) gid = va_arg(argp, long);
		va_end(argp);
	}

	while ((fn = HXdir_read(dt)) != NULL) {
		char fsrc[MAXFNLEN], fdest[MAXFNLEN];
		struct stat sb;

		if (strcmp(fn, ".") == 0 || strcmp(fn, "..") == 0)
			continue;
		snprintf(fsrc,  MAXFNLEN, "%s/%s", src,  fn);
		snprintf(fdest, MAXFNLEN, "%s/%s", dest, fn);
		if (lstat(fsrc, &sb) < 0)
			continue;
		sb.st_mode &= 0777; /* clear SUID/GUID/Sticky bits */

		if (S_ISREG(sb.st_mode)) {
			HX_copy_file(fsrc, fdest, opts, uid, gid);
		} else if (S_ISDIR(sb.st_mode)) {
			HX_mkdir(fdest, S_IRWXUGO);
			HX_copy_dir(fsrc, fdest, opts, uid, gid);
		} else if (S_ISLNK(sb.st_mode)) {
			char pt[MAXFNLEN];
			memset(pt, '\0', MAXFNLEN);
			if (readlink(fsrc, pt, MAXFNLEN - 1) < MAXFNLEN - 1)
				if (symlink(pt, fdest) < 0)
					/* ignore */;
		} else if (S_ISBLK(sb.st_mode) || S_ISCHR(sb.st_mode)) {
			if (mknod(fdest, sb.st_mode, sb.st_dev) < 0)
				/* ignore */;
		} else if (S_ISFIFO(sb.st_mode)) {
			if (mkfifo(fdest, sb.st_mode) < 0)
				/* ignore */;
		}

		if (lchown(fdest, uid, gid) < 0)
			/* ignore */;
		if (!S_ISLNK(sb.st_mode))
			if (chmod(fdest, sb.st_mode) < 0)
				/* ignore */;
	}

	HXdir_close(dt);
	return 1;
}

EXPORT_SYMBOL int HX_mkdir(const char *idir, unsigned int mode)
{
	int i = 0, len = strlen(idir);
	char buf[MAXFNLEN], dir[MAXFNLEN];

#if defined(_WIN32)
	{
		char *p = dir;
		HX_strlcpy(dir, idir, sizeof(dir));
		for (; *p != '\0'; ++p)
			if (*p == '\\')
				*p = '/';
		if (HX_isalpha(dir[0]) && dir[1] == ':')
			i = 2;
	}
#else
	HX_strlcpy(dir, idir, sizeof(dir));
#endif

	if (dir[i] == '/')
		++i;
	for (; i < len; ++i) {
		int v;
		if (dir[i] == '/') {
			strncpy(buf, dir, i);
			buf[i] = '\0';
			if ((v = mkdir_gen(buf, mode)) < 0)
				return v;
		} else if (i == len - 1) {
			strncpy(buf, dir, len);
			buf[len] = '\0';
			if ((v = mkdir_gen(buf, mode)) < 0)
				return v;
		}
	}
	return 1;
}

/* Readlink - with a trailing zero (provided by HXmc) */
EXPORT_SYMBOL int HX_readlink(hxmc_t **target, const char *path)
{
	bool allocate = *target == NULL;
	size_t linkbuf_size;

	if (allocate) {
		linkbuf_size = 128;
		*target = HXmc_meminit(nullptr, 128);
		if (*target == NULL)
			return -errno;
	} else {
		linkbuf_size = HXmc_length(*target);
		if (linkbuf_size < 128) {
			linkbuf_size = 128;
			if (HXmc_setlen(target, 128) == nullptr)
				return -errno;
		}
	}
	while (true) {
		ssize_t ret = readlink(path, *target, linkbuf_size);
		if (ret < 0) {
			int saved_errno = errno;
			if (allocate) {
				HXmc_free(*target);
				*target = nullptr;
			}
			return -(errno = saved_errno);
		}
		if (static_cast(size_t, ret) < linkbuf_size) {
			(*target)[ret] = '\0'; // please cov-scan
			HXmc_setlen(target, ret); // \0 set here anyway
			return ret;
		}
		if (linkbuf_size > SIZE_MAX / 2)
			return -E2BIG;
		linkbuf_size *= 2;
		if (HXmc_setlen(target, linkbuf_size) == NULL) {
			int saved_errno = errno;
			if (allocate) {
				HXmc_free(*target);
				*target = nullptr;
			}
			return -(errno = saved_errno);
		}
	}
	return 0;
}

/**
 * The buffers HX_realpath_symres are used are retained across symres calls to
 * not do unnecessarily many allocation calls. Downside is that the state is
 * roughly 12K in the worst case.
 */
struct HX_realpath_state {
	hxmc_t *dest;
	hxmc_t *link_target;
	hxmc_t *new_path;
	hxmc_t *symres_tmp;
	const char *path;
	unsigned int deref_count;
};

/**
 * Perform symlink resolution on the currently last component (state->dest).
 */
static int
HX_realpath_symres(struct HX_realpath_state *state, const char *path)
{
	int ret;

	ret = HX_readlink(&state->link_target, state->dest);
	if (ret == -EINVAL)
		return -EINVAL;
	else if (ret < 0)
		return -errno;
#ifdef __MINGW32__
	else if (state->deref_count++ >= 40)
		/*
		 * not that this is ever going to happen on mingw,
		 * because Windows does not seem to have symlinks, ...
		 */
		return -EINVAL;
#else
	else if (state->deref_count++ >= 40)
		return -ELOOP;
#endif

	assert(state->link_target != nullptr);
	if (*state->link_target == '/') {
		*state->dest = '\0';
		if (HXmc_setlen(&state->dest, 0) == NULL)
			return -errno;
	} else {
		char *dptr = state->dest + HXmc_length(state->dest);
		while (*--dptr != '/')
			;
		*dptr = '\0';
		if (HXmc_setlen(&state->dest, dptr - state->dest) == NULL)
			return -errno;
	}

	if (HXmc_strcpy(&state->symres_tmp, state->link_target) == NULL)
		return -errno;
	/*
	 * @path could be pointing to @state->new_path already, so we need
	 * to construct the new path in a temp buffer (@symres_tmp) first.
	 */
	if (HXmc_strcat(&state->symres_tmp, path) == NULL)
		return -errno;
	if (HXmc_strcpy(&state->new_path, state->symres_tmp) == NULL)
		return -errno;
	state->path = state->new_path;
	return 1;
}

EXPORT_SYMBOL int HX_getcwd(hxmc_t **target)
{
	bool allocate = *target == nullptr;
	size_t linkbuf_size;

	if (allocate) {
		linkbuf_size = 128;
		*target = HXmc_meminit(nullptr, linkbuf_size);
		if (*target == nullptr)
			return -errno;
	} else {
		linkbuf_size = HXmc_length(*target);
		if (linkbuf_size < 128) {
			linkbuf_size = 128;
			if (HXmc_setlen(target, linkbuf_size) == nullptr)
				return -errno;
		}
	}
	while (true) {
		const char *ret = getcwd(*target, linkbuf_size);
		if (ret != nullptr) {
			HXmc_setlen(target, strlen(ret)); /* shrink to fit */
			return 1;
		}
		if (errno == ERANGE) {
			if (linkbuf_size > SIZE_MAX / 2)
				return -E2BIG;
			linkbuf_size *= 2;
			if (HXmc_setlen(target, linkbuf_size) != nullptr)
				continue;
			/* errno already set by realloc, fall into next if block */
		}
		int saved_errno = errno;
		if (allocate) {
			HXmc_free(*target);
			*target = nullptr;
		}
		return -(errno = saved_errno);
	}
	return -EINVAL;
}

EXPORT_SYMBOL int HX_realpath(hxmc_t **dest_pptr, const char *path,
    unsigned int flags)
{
	struct HX_realpath_state state = {.dest = *dest_pptr};
	bool rq_slash = false, dnull = state.dest == NULL;
	const char *cptr, *orig_path = path;
	int ret = 0;

	if (dnull) {
		state.dest = HXmc_meminit(NULL, 256);
		if (state.dest == NULL)
			goto err;
	}

	if (*path == '/') {
		rq_slash = true;
	} else if (flags & HX_REALPATH_ABSOLUTE) {
		if (HX_getcwd(&state.dest) < 0)
			goto err;
		rq_slash = true;
	}

	while (*path != '\0') {
		if (*path == '/') {
			++path;
			continue;
		} else if (path[0] == '.' &&
		    (path[1] == '/' || path[1] == '\0') &&
		    flags & HX_REALPATH_SELF) {
			++path;
			continue;
		} else if (path[0] == '.' && path[1] == '.' &&
		    (path[2] == '/' || path[2] == '\0') &&
		    flags & HX_REALPATH_PARENT &&
		    ((flags & HX_REALPATH_ABSOLUTE) || *state.dest != '\0')) {
			cptr  = state.dest + HXmc_length(state.dest);
			path += 2;
			while (cptr > state.dest && *--cptr != '/')
				;
			state.dest[cptr-state.dest] = '\0';
			if (HXmc_setlen(&state.dest,
			    cptr - state.dest) == NULL)
				goto err;
			continue;
		}

		for (cptr = path; *cptr != '\0' && *cptr != '/'; ++cptr)
			;
		if (rq_slash && HXmc_strcat(&state.dest, "/") == NULL)
			goto out;
		if (HXmc_memcat(&state.dest, path, cptr - path) == NULL)
			goto out;
		path = cptr;
		rq_slash = true;

		ret = HX_realpath_symres(&state, path);
		if (ret == -EINVAL)
			continue;
		else if (ret <= 0)
			goto out;
		path = state.path;
	}

	if (*state.dest == '\0') {
		if (*orig_path == '/') {
			if (HXmc_strcpy(&state.dest, "/") == NULL)
				goto err;
		} else {
			if (HXmc_strcpy(&state.dest, ".") == NULL)
				goto err;
		}
	}

	*dest_pptr = state.dest;
	HXmc_free(state.link_target);
	HXmc_free(state.new_path);
	HXmc_free(state.symres_tmp);
	return 1;

 err:
	ret = -errno;
 out:
	if (dnull) {
		/* If caller supplied a buffer, do not take it away. */
		HXmc_free(state.dest);
		*dest_pptr = NULL;
	} else {
		*dest_pptr = state.dest;
	}
	HXmc_free(state.link_target);
	HXmc_free(state.new_path);
	HXmc_free(state.symres_tmp);
	return ret;
}

EXPORT_SYMBOL int HX_rrmdir(const char *dir)
{
	struct HXdir *ptr;
	const char *trav;
	hxmc_t *fn = NULL;
	int ret = 0;

	if ((ptr = HXdir_open(dir)) == NULL)
		return -errno;

	while ((trav = HXdir_read(ptr)) != NULL) {
		struct stat sb;

		if (strcmp(trav, ".") == 0 || strcmp(trav, "..") == 0)
			continue;
		HXmc_strcpy(&fn, dir);
		HXmc_strcat(&fn, "/");
		HXmc_strcat(&fn, trav);
		if (lstat(fn, &sb) < 0) {
			if (ret == 0)
				ret = -errno;
			continue;
		}

		if (S_ISDIR(sb.st_mode)) {
			if (HX_rrmdir(fn) <= 0) {
				if (ret == 0)
					ret = -errno;
				continue;
			}
		} else if (unlink(fn) < 0) {
			if (ret == 0)
				ret = -errno;
			continue;
		}
	}

	if (rmdir(dir) < 0) {
		if (ret == 0)
			ret = -errno;
	}
	HXdir_close(ptr);
	HXmc_free(fn);
	return ret;
}

EXPORT_SYMBOL ssize_t HXio_fullread(int fd, void *vbuf, size_t size)
{
	char *buf = vbuf;
	if (size > SSIZE_MAX)
		size = SSIZE_MAX;

	while (size > 0) {
		ssize_t ret = read(fd, buf, size);
		if (ret < 0)
			return ret;
		else if (ret == 0)
			break;
		buf += ret;
		size -= ret;
	}
	return buf - static_cast(char *, vbuf);
}

EXPORT_SYMBOL ssize_t HXio_fullwrite(int fd, const void *vbuf, size_t size)
{
	const char *buf = vbuf;
	if (size > SSIZE_MAX)
		size = SSIZE_MAX;

	while (size > 0) {
		ssize_t ret = write(fd, buf, size);
		if (ret < 0)
			return ret;
		else if (ret == 0)
			break;
		buf += ret;
		size -= ret;
	}
	return buf - static_cast(const char *, vbuf);
}

#if __linux__
#ifdef HAVE_COPY_FILE_RANGE
static ssize_t HX_cfr_linux(int dst, int src, size_t count)
{
	ssize_t ret, xferd = 0;
	/*
	 * Use INT(32)_MAX rather than SSIZE_MAX, as there is an issue with
	 * overflow detection pending.
	 * https://lore.kernel.org/linux-man/38nr2286-1o9q-0004-2323-799587773o15@vanv.qr/
	 */
	size_t xfersize = INT_MAX;
	if (count > xfersize)
		count = xfersize;
	while ((ret = copy_file_range(src, nullptr, dst, nullptr, count, 0)) > 0)
		xferd += ret;
	if (xferd > 0)
		return xferd;
	if (ret < 0)
		return -errno;
	return 0;
}
#endif

static ssize_t HX_sendfile_linux(int dst, int src, size_t count)
{
	ssize_t ret, xferd = 0;
	/*
	 * Use INT(32)_MAX rather than SSIZE_MAX, as there is an issue with
	 * overflow detection pending.
	 * https://lore.kernel.org/linux-man/38nr2286-1o9q-0004-2323-799587773o15@vanv.qr/
	 */
	size_t xfersize = INT_MAX;
	if (count > xfersize)
		count = xfersize;
	while ((ret = sendfile(dst, src, nullptr, count)) > 0)
		xferd += ret;
	if (xferd > 0)
		return xferd;
	if (ret < 0)
		return -errno;
	return 0;
}
#endif

static ssize_t HX_sendfile_rw(int dst, int src, size_t count)
{
	static const size_t bufsize = 0x10000;
	size_t xferd = 0;
	ssize_t ret = 0;
	void *buf = malloc(bufsize);
	if (buf == nullptr)
		return -ENOMEM;
	if (count > SSIZE_MAX)
		count = SSIZE_MAX;
	while (count > 0) {
		size_t readsize = bufsize;
		if (count < readsize)
			readsize = count;
		/* Return value of fullread/write is same as read/write(2). */
		ret = HXio_fullread(src, buf, readsize);
		if (ret < 0)
			break;
		ret = HXio_fullwrite(dst, buf, ret);
		if (ret < 0)
			break;
		xferd += ret;
		count -= ret;
	}
	free(buf);
	if (xferd > 0)
		return xferd;
	if (ret < 0)
		return -errno;
	return 0;
}

EXPORT_SYMBOL ssize_t HX_sendfile(int dst, int src, size_t count)
{
#if __linux__
	ssize_t ret;
#ifdef HAVE_COPY_FILE_RANGE
	ret = HX_cfr_linux(dst, src, count);
	if (ret != -ENOSYS && ret != -EXDEV)
		return ret;
#endif
	ret = HX_sendfile_linux(dst, src, count);
	if (ret != -ENOSYS)
		return ret;
#endif
	return HX_sendfile_rw(dst, src, count);
}

EXPORT_SYMBOL char *HX_slurp_fd(int fd, size_t *outsize)
{
	struct stat sb;
	if (fstat(fd, &sb) < 0)
		return NULL;
	if (sb.st_size == 0) {
		/* e.g. ttys (S_ISCHR) or special procfs files */
		size_t bufsize = 4096, offset = 0;
		char *buf = malloc(bufsize);
		if (buf == nullptr)
			return nullptr;
		do {
			assert(offset < bufsize);
			ssize_t rdret = read(fd, buf + offset, bufsize - 1 - offset);
			if (rdret <= 0)
				break;
			offset += rdret;
			if (bufsize - offset >= 4095)
				/* 4K room still, just continue reading. */
				continue;

			/* Less than 4K room, enlarge now */
			if (bufsize > SIZE_MAX / 2)
				/* No more doubling */
				break;
			bufsize *= 2;
			void *nbuf = realloc(buf, bufsize + 1);
			if (nbuf == nullptr) {
				int se = errno;
				free(buf);
				errno = se;
				return nullptr;
			}
			buf = nbuf;
		} while (true);
		buf[offset] = '\0';
		if (outsize != nullptr)
			*outsize = offset;
		return buf;
	}
	size_t fsize = sb.st_size; /* may truncate from loff_t to size_t */
	if (fsize == SIZE_MAX)
		--fsize;
#ifdef HAVE_POSIX_FADVISE
	if (fsize > 0 && posix_fadvise(fd, 0, fsize,
	    POSIX_FADV_SEQUENTIAL) != 0)
		/* ignore */;
#endif
	char *buf = malloc(fsize + 1);
	if (buf == NULL)
		return NULL;
	ssize_t rdret = HXio_fullread(fd, buf, fsize);
	if (rdret < 0) {
		int se = errno;
		free(buf);
		errno = se;
		return NULL;
	}
	buf[rdret] = '\0';
	if (outsize != NULL)
		*outsize = rdret;
	return buf;
}

EXPORT_SYMBOL char *HX_slurp_file(const char *file, size_t *outsize)
{
	int fd = open(file, O_RDONLY | O_BINARY | O_CLOEXEC);
	if (fd < 0)
		return NULL;
	size_t tmpsize;
	if (outsize == NULL)
		outsize = &tmpsize;
	char *buf = HX_slurp_fd(fd, outsize);
	if (buf == NULL) {
		int se = errno;
		close(fd);
		errno = se;
		return NULL;
	}
	close(fd);
	return buf;
}
