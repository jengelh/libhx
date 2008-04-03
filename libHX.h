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
 *	HMC.C
 */
#ifndef __libhx_internal_hmc_t_defined
#define __libhx_internal_hmc_t_defined 1
typedef char hmc_t;
#endif
hmc_t *hmc_dup(const void *);
hmc_t *hmc_sinit(const char *);
hmc_t *hmc_minit(const void *, size_t);
hmc_t *hmc_strasg(hmc_t **, const char *);
hmc_t *hmc_memasg(hmc_t **, const void *, size_t);
size_t hmc_length(hmc_t *);
hmc_t *hmc_trunc(hmc_t **, size_t);
hmc_t *hmc_strcat(hmc_t **, const char *);
hmc_t *hmc_memcat(hmc_t **, const void *, size_t);
hmc_t *hmc_strpcat(hmc_t **, const char *);
hmc_t *hmc_mempcat(hmc_t **, const void *, size_t);
hmc_t *hmc_strins(hmc_t **, size_t, const char *);
hmc_t *hmc_memins(hmc_t **, size_t, const void *, size_t);
hmc_t *hmc_memdel(hmc_t *, size_t, size_t);
void hmc_free(hmc_t *);

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

/*
 *	STRING.C
 */
extern char *HX_basename(const char *);
extern char *HX_chomp(char *);
extern char *HX_dirname(const char *);
extern hmc_t *HX_getl(hmc_t **, FILE *);
extern char **HX_split(const char *, const char *, int *, int);
extern int HX_split5(char *, const char *, int, char **);
extern char *HX_strbchr(const char *, const char *, char);
extern char *HX_strclone(char **, const char *);
extern char *HX_strlower(char *);
extern size_t HX_strltrim(char *);
extern char *HX_strmid(const char *, long, long);
extern size_t HX_strrcspn(const char *, const char *);
extern char *HX_strrev(char *);
extern size_t HX_strrtrim(char *);
extern char *HX_strsep(char **, const char *);
extern char *HX_strsep2(char **, const char *);
extern char *HX_strupper(char *);

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
static inline void *HX_memdup(const void *buf, size_t len)
{
	void *ret;
	if ((ret = malloc(len)) == NULL)
		return NULL;
	return memcpy(ret, buf, len);
}

static inline char *HX_strlcat(char *dest, const char *src, size_t len)
{
	ssize_t x = len - strlen(dest) - 1;
	if (x <= 0)
		return dest;
	return strncat(dest, src, x);
}

static inline char *HX_strlcpy(char *dest, const char *src, size_t n)
{
	strncpy(dest, src, n);
	dest[n-1] = '\0';
	return dest;
}

static inline char *HX_strlncat(char *dest, const char *src, size_t dlen,
    size_t slen)
{
	ssize_t x = dlen - strlen(dest) - 1;
	if (x <= 0)
		return dest;
	x = ((ssize_t)slen < x) ? (ssize_t)slen : x;
	return strncat(dest, src, x);
}

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
HX_memdup(const void *data, size_t n)
{
	return reinterpret_cast<type>(HX_memdup(data, n));
}

template<typename type> static inline type
HX_dlsym(void *handle, const char *symbol)
{
	return reinterpret_cast<type>(HX_dlsym(handle, symbol));
}

} /* extern "C++" */
#endif

#ifdef __cplusplus
extern "C" {
#endif

static inline char *HX_strdup(const char *src)
{
	if (src == NULL)
		return NULL;
#ifdef __cplusplus
	return HX_memdup<char *>(src, strlen(src) + 1);
#else
	return HX_memdup(src, strlen(src) + 1);
#endif
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _LIBHX_H */
