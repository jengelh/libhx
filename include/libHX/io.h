#ifndef _LIBHX_IO_H
#define _LIBHX_IO_H 1

#include <sys/types.h>

enum {
	HXF_UID  = 1 << 0,
	HXF_GID  = 1 << 1,
	HXF_KEEP = 1 << 2,
};

struct HXdir;

extern struct HXdir *HXdir_open(const char *);
extern const char *HXdir_read(struct HXdir *);
extern void HXdir_close(struct HXdir *);
extern int HX_copy_dir(const char *, const char *, unsigned int, ...);
extern int HX_copy_file(const char *, const char *, unsigned int, ...);
extern int HX_mkdir(const char *);
extern int HX_rrmdir(const char *);

extern ssize_t HXio_fullread(int, void *, size_t);
extern ssize_t HXio_fullwrite(int, const void *, size_t);

#endif /* _LIBHX_IO_H */
