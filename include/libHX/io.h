#ifndef _LIBHX_IO_H
#define _LIBHX_IO_H 1

#include <stdio.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __libhx_internal_hxmc_t_defined
#define __libhx_internal_hxmc_t_defined 1
typedef char hxmc_t;
#endif

enum {
	HXF_UID  = 1 << 0,
	HXF_GID  = 1 << 1,
	HXF_KEEP = 1 << 2,
	HX_REALPATH_NOFLAGS  = 0,
	HX_REALPATH_ABSOLUTE = 1 << 0,
	HX_REALPATH_SELF     = 1 << 1,
	HX_REALPATH_PARENT   = 1 << 2,
	/* HX_REALPATH_SYMLINK  = 1 << 3, removed in v3.13, thus blocked */
	HX_REALPATH_DEFAULT  = HX_REALPATH_SELF | HX_REALPATH_PARENT,
};

struct HXdir;

extern struct HXdir *HXdir_open(const char *);
extern const char *HXdir_read(struct HXdir *);
extern void HXdir_close(struct HXdir *);
extern int HX_copy_dir(const char *, const char *, unsigned int, ...);
extern int HX_copy_file(const char *, const char *, unsigned int, ...);
extern int HX_mkdir(const char *, unsigned int);
extern int HX_getcwd(hxmc_t **);
extern int HX_readlink(hxmc_t **, const char *);
extern int HX_realpath(hxmc_t **, const char *, unsigned int);
extern int HX_rrmdir(const char *);
extern ssize_t HX_sendfile(int dst, int src, size_t count);
extern char *HX_slurp_fd(int fd, size_t *outsize);
extern char *HX_slurp_file(const char *file, size_t *outsize);

extern ssize_t HXio_fullread(int, void *, size_t);
extern ssize_t HXio_fullwrite(int, const void *, size_t);
#ifndef HX_HEXDUMP_DECLARATION
#define HX_HEXDUMP_DECLARATION 1
extern void HX_hexdump(FILE *, const void *, unsigned int);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _LIBHX_IO_H */
