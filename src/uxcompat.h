#ifndef _LIBHX_UXCOMPAT_H
#define _LIBHX_UXCOMPAT_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/stat.h>

#ifndef ENOSYS
#	define ENOSYS 38 /* Function not implemented */
#endif

#ifndef S_IFLNK
#	define S_IFLNK  0xA000
#endif
#ifndef S_IFSOCK
#	define S_IFSOCK 0xC000
#endif
#ifndef S_IFBLK
#	define S_IFBLK 0x6000
#endif
#ifndef S_IFCHR
#	define S_IFCHR 0x2000
#endif
#ifndef S_IFIFO
#	define S_IFIFO 0x1000
#endif
#ifndef S_ISBLK
#	define S_ISBLK(__mode) (((__mode) & S_IFMT) == S_IFBLK)
#endif
#ifndef S_ISCHR
#	define S_ISCHR(__mode) (((__mode) & S_IFMT) == S_IFCHR)
#endif
#ifndef S_ISDIR
#	define S_ISDIR(__mode) (((__mode) & S_IFMT) == S_IFDIR)
#endif
#ifndef S_ISREG
#	define S_ISREG(__mode) (((__mode) & S_IFMT) == S_IFREG)
#endif
#ifndef S_ISLNK
#	define S_ISLNK(__mode) (((__mode) & S_IFMT) == S_IFLNK)
#endif
#ifndef S_ISFIFO
#	define S_ISFIFO(__mode) (((__mode) & S_IFMT) == S_IFIFO)
#endif
#ifndef S_ISSOCK
#	define S_ISSOCK(__mode) (((__mode) & S_IFMT) == S_IFSOCK)
#endif

/*
 *	UX-FILE.C
 */
extern int chown(const char *, long, long);
extern int fchmod(int, long);
extern int fchown(int, long, long);
extern int lchown(const char *, long, long);
extern int lstat(const char *, struct stat *);
extern int mkfifo(const char *, long);
extern int mknod(const char *, long, long);
extern int readlink(const char *, char *, size_t);
extern int symlink(const char *, const char *);

/*
 *	UX-MMAP.C
 */
extern void *mmap(void *, size_t, int, int, int, off_t);
extern int munmap(void *, size_t);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _LIBHX_UXCOMPAT_H */
