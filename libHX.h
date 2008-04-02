/*
 *	libHX/libHX.h
 *	Copyright © Jan Engelhardt <jengelh [at] gmx de>, 1999 - 2008
 *
 *	This file is part of libHX. libHX is free software; you can
 *	redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2 or 3 of the License.
 */
#ifndef _LIBHX_H
#define _LIBHX_H 20080402

#ifndef __cplusplus
#	include <stdarg.h>
#	include <stdio.h>
#	include <stdlib.h>
#	include <string.h>
#else
#	include <cstdarg>
#	include <cstdio>
#	include <cstdlib>
#	include <cstring>
#endif
#if defined _WIN32
#	include <windows.h>
#else
#	include <dirent.h>
#	include <dlfcn.h>
#	include <unistd.h>
#endif
#ifdef LIBHX_INTERNAL /* only for compiling libHX */
#	include "libHX/internal.h"
#endif

#include <libHX/arbtree.h>
#include <libHX/deque.h>
#include <libHX/option.h>
#include <libHX/string.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *	DIR.C
 */
enum {
	HXF_UID  = 1 << 0,
	HXF_GID  = 1 << 1,
	HXF_KEEP = 1 << 2,
};

extern void *HXdir_open(const char *);
extern const char *HXdir_read(void *);
extern void HXdir_close(void *);
extern int HX_copy_dir(const char *, const char *, unsigned int, ...);
extern int HX_copy_file(const char *, const char *, unsigned int, ...);
extern int HX_mkdir(const char *);
extern int HX_rrmdir(const char *);

/*
 *	DL.C
 */
extern void *HX_dlopen(const char *);
extern void *HX_dlsym(void *, const char *);
extern void HX_dlclose(void *);
extern const char *HX_dlerror(void);

/*
 *	OTHER.C
 */
enum {
	HX_FSYSTEM_ARGV  = 1 << 0,
	HX_FSYSTEM_EXEC  = 1 << 1,
	HX_FSYSTEM_ARGV1 = 1 << 2,
};

extern void HX_zvecfree(char **);
extern int HX_fsystem(unsigned int, const char *, const char *, ...);
extern int HX_vfsystem(unsigned int, const char *, const char *, va_list);

/*
 *	RAND.C
 */
extern int HX_rand(void);
extern unsigned int HX_irand(unsigned int, unsigned int);

#ifdef _WIN32
#	define MAP_FAILED ((void *)-1)
#	define PROT_NONE   0x0
#	define PROT_READ   0x1
#	define PROT_WRITE  0x2
#	define PROT_EXEC   0x4
#	define MAP_SHARED  0x1
#	define MAP_PRIVATE 0x2
extern void *mmap(void *, size_t, int, int, int, off_t);
extern int munmap(void *, size_t);
#endif

/*
 *	INLINE FUNCTIONS
 */
static inline int HX_zveclen(const char **args)
{
	int argk = 0;
	while (*args++ != NULL)
		++argk;
	return argk;
}

#ifdef __cplusplus
} /* extern "C" */

extern "C++" {

template<typename type> static inline type
HX_dlsym(void *handle, const char *symbol)
{
	return reinterpret_cast<type>(HX_dlsym(handle, symbol));
}

} /* extern "C++" */
#endif

#endif /* _LIBHX_H */
