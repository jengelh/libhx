#ifndef _LIBHX_IO_H
#define _LIBHX_IO_H 1

#include <sys/types.h>

extern ssize_t HXio_fullread(int, void *, size_t);
extern ssize_t HXio_fullwrite(int, const void *, size_t);

#endif /* _LIBHX_IO_H */
