/*
	libHX/libHX.h
	Copyright © Jan Engelhardt <jengelh [at] gmx de>, 1999 - 2007

	This file is part of libHX. libHX is free software; you can
	redistribute it and/or modify it under the terms of the GNU
	Lesser General Public License as published by the Free Software
	Foundation; however ONLY version 2 of the License. For details,
	see the file named "LICENSE.LGPL2".
*/
#ifndef _LIBHX_H
#define _LIBHX_H 20070330

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
#	include "internal.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 *	ARBTREE.C
 */
enum {
	/* activates key=>value pairs */
	HXBT_MAP   = 1 << 0,
	/* copy key  (only for string keys!) */
	HXBT_CKEY  = 1 << 1,
	/* copy data (only for string data!) */
	HXBT_CDATA = 1 << 2,
	/* pointer to comparison routine passed */
	HXBT_CMPFN = 1 << 3,
	/* use direct integer comparison */
	HXBT_ICMP  = 1 << 4,
	/* use strcmp() -- abbreviation for HXBT_CMPFN,strcmp */
	HXBT_SCMP  = 1 << 5,
	/* use CIDs for traverser */
	HXBT_CID   = 1 << 6,
};

struct HXbtree_node {
	struct HXbtree_node *s[2];
	void *key, *data;
	unsigned char color;
};

struct HXbtree {
	struct HXbtree_node *root;
	unsigned long opts, itemcount;
	unsigned int transact;
	int (*cmpfn)(const void *, const void *);
	void *uptr;
};

extern struct HXbtree *HXbtree_init(unsigned long, ...);
extern struct HXbtree_node *HXbtree_add(struct HXbtree *, const void *, ...);
extern struct HXbtree_node *HXbtree_find(const struct HXbtree *, const void *);
extern void *HXbtree_get(const struct HXbtree *, const void *);
extern void *HXbtree_del(struct HXbtree *, const void *);
extern void HXbtree_free(struct HXbtree *);
extern void *HXbtrav_init(const struct HXbtree *);
extern struct HXbtree_node *HXbtraverse(void *);
extern void HXbtrav_free(void *);

/*
 *	DEQUE.C
 */
struct HXdeque_node {
	struct HXdeque_node *next, *prev;
	struct HXdeque *parent;
	void *ptr;
};

struct HXdeque {
	struct HXdeque_node *first, *last;
	unsigned long itemcount;
	void *ptr;
};

extern struct HXdeque *HXdeque_init(void);
extern struct HXdeque_node *HXdeque_push(struct HXdeque *, const void *);
extern struct HXdeque_node *HXdeque_unshift(struct HXdeque *, const void *);
extern void *HXdeque_pop(struct HXdeque *);
extern void *HXdeque_shift(struct HXdeque *);
extern void HXdeque_move(struct HXdeque_node *, struct HXdeque_node *);
extern struct HXdeque_node *HXdeque_find(struct HXdeque *, const void *);
extern void *HXdeque_get(struct HXdeque *, const void *);
extern void *HXdeque_del(struct HXdeque_node *);
extern void HXdeque_free(struct HXdeque *);
extern void HXdeque_genocide(struct HXdeque *);
extern void **HXdeque_to_vec(struct HXdeque *, long *);

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
extern int HX_copy_dir(const char *, const char *, unsigned long, ...);
extern int HX_copy_file(const char *, const char *, unsigned long, ...);
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
typedef char hmc_t;
hmc_t *hmc_dup(const void *);
hmc_t *hmc_sinit(const char *);
hmc_t *hmc_minit(const void *, long);
hmc_t *hmc_strasg(hmc_t **, const char *);
hmc_t *hmc_memasg(hmc_t **, const void *, long);
long hmc_length(hmc_t *);
hmc_t *hmc_trunc(hmc_t **, long);
hmc_t *hmc_strcat(hmc_t **, const char *);
hmc_t *hmc_memcat(hmc_t **, const void *, long);
hmc_t *hmc_strpcat(hmc_t **, const char *);
hmc_t *hmc_mempcat(hmc_t **, const void *, long);
hmc_t *hmc_strins(hmc_t **, long, const char *);
hmc_t *hmc_memins(hmc_t **, long, const void *, long);
hmc_t *hmc_memdel(hmc_t *, long, long);
void hmc_free(hmc_t *);

/*
 *	FORMAT.C
 */
struct HXoption;
extern struct HXbtree *HXformat_init(void);
extern void HXformat_free(struct HXbtree *);
extern int HXformat_add(struct HXbtree *, const char *, const void *,
    unsigned int);
extern int HXformat_aprintf(const struct HXbtree *, hmc_t **, const char *);
extern int HXformat_sprintf(const struct HXbtree *, char *, size_t, const char *);
extern int HXformat_fprintf(const struct HXbtree *, FILE *, const char *);

/*
 *	OPT.C
 */
enum {
	/* .type */
	HXTYPE_NONE = 0,
	/* for opt: set specific integer value */
	HXTYPE_VAL,
	/* for opt: set specific string value */
	HXTYPE_SVAL,
	/* accept a string "yes", "no", "true", "false" and put into *(int*) */
	HXTYPE_BOOL,
	/* read _one byte_ and put it into *(char *) */
	HXTYPE_BYTE,
	/* read an integer/float (sscanf %d/%o/%x/%f) */
	HXTYPE_UCHAR,
	HXTYPE_CHAR,
	HXTYPE_USHORT,
	HXTYPE_SHORT,
	HXTYPE_UINT,
	HXTYPE_INT,
	HXTYPE_ULONG,
	HXTYPE_LONG,
	HXTYPE_ULLONG,
	HXTYPE_LLONG,
	HXTYPE_FLOAT,
	HXTYPE_DOUBLE,
	/* read string and put into *(const char **) */
	HXTYPE_STRING,
	HXTYPE_STRP, /* (const char **) */
	HXTYPE_STRDQ,

	/* .type extra flags */
	/* argument is optional */
	HXOPT_OPTIONAL = 1 << 6,
	/* increase pointed variable */
	HXOPT_INC      = 1 << 7,
	/* decrease pointed variable */
	HXOPT_DEC      = 1 << 8,
	/* negate input first */
	HXOPT_NOT      = 1 << 9,
	/* or pointed variable with input */
	HXOPT_OR       = 1 << 10,
	/* and pointed variable with input */
	HXOPT_AND      = 1 << 11,
	/* xor pointed variable with input */
	HXOPT_XOR      = 1 << 12,
	HXFORMAT_IMMED = 1 << 13,

	HXOPT_LOPMASK2 = HXOPT_OR | HXOPT_AND | HXOPT_XOR,
	HXOPT_LOPMASK  = HXOPT_LOPMASK2 | HXOPT_NOT,
	HXOPT_TYPEMASK = 0x1F, /* 5 bits */

	/* HX_getopt() flags */
	HXOPT_PTHRU       = 1 << 0,
	HXOPT_DESTROY_OLD = 1 << 1,
	HXOPT_QUIET       = 1 << 2,
	HXOPT_HELPONERR   = 1 << 3,
	HXOPT_USAGEONERR  = 1 << 4,

	/* Return types for HX_getopt() */
	HXOPT_ERR_UNKN = 1,
	HXOPT_ERR_VOID,
	HXOPT_ERR_MIS,

	SHCONF_ONE = 1 << 0, /* only read one configuration file */
};

struct HXoption;
struct HXoptcb {
	const char *arg0;
	const struct HXoption *table, *current;
	const char *s;
	double d;
	long l;
	const char *tln;
	char tsh;
};

struct HXoption {
	const char *ln;
	char sh;
	unsigned int type;
	void *ptr, *uptr;
	void (*cb)(const struct HXoptcb *);
	int val;
	const char *sval, *help, *htyp;
};

extern int HX_getopt(const struct HXoption *, int *, const char ***,
	unsigned int);
extern void HX_getopt_help(const struct HXoptcb *, FILE *);
extern void HX_getopt_help_cb(const struct HXoptcb *);
extern void HX_getopt_usage(const struct HXoptcb *, FILE *);
extern void HX_getopt_usage_cb(const struct HXoptcb *);
extern int HX_shconfig(const char *, const struct HXoption *);
extern int HX_shconfig_pv(const char **, const char *,
	const struct HXoption *, unsigned long);
extern void HX_shconfig_free(const struct HXoption *);

#ifndef __cplusplus
#	define HXOPT_AUTOHELP \
		{.ln = "help", .sh = '?', .type = HXTYPE_NONE, \
		.cb = HX_getopt_help_cb, .help = "Show this help message"}, \
		{.ln = "usage", .type = HXTYPE_NONE, \
		.cb = HX_getopt_usage_cb, \
		.help = "Display brief usage message"}
#	define HXOPT_TABLEEND {.ln = NULL, .sh = 0}
#else
#	define HXOPT_AUTOHELP \
		{NULL, '?', HXTYPE_NONE, NULL, NULL, HX_getopt_help_cb, \
		0, NULL, "Show this help message"}
#	define HXOPT_TABLEEND {NULL, 0}
#endif

/*
 *	OTHER.C
 */
enum {
	HX_FSYSTEM_ARGV  = 1 << 0,
	HX_FSYSTEM_EXEC  = 1 << 1,
	HX_FSYSTEM_ARGV1 = 1 << 2,
};

extern int HX_ffs(unsigned long);
extern void HX_zvecfree(char **);
extern int HX_fsystem(unsigned long, const char *, const char *, ...);
extern int HX_vfsystem(unsigned long, const char *, const char *, va_list);

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
	if((ret = malloc(len)) == NULL)
		return NULL;
	return memcpy(ret, buf, len);
}

static inline char *HX_strlcat(char *dest, const char *src, size_t len)
{
	ssize_t x = len - strlen(dest) - 1;
	if(x <= 0)
		return dest;
	return strncat(dest, src, x);
}

static inline char *HX_strlcpy(char *dest, const char *src, size_t n)
{
	strncpy(dest, src, n);
	dest[n - 1] = '\0';
	return dest;
}

static inline char *HX_strlncat(char *dest, const char *src, size_t dlen,
    size_t slen)
{
	ssize_t x = dlen - strlen(dest) - 1;
	if(x <= 0)
		return dest;
	x = ((ssize_t)slen < x) ? (ssize_t)slen : x;
	return strncat(dest, src, x);
}

static inline int HX_zveclen(const char **args)
{
	int argk = 0;
	while(*args++ != NULL)
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
HXbtree_get(struct HXbtree *bt, const void *ptr)
{
	return reinterpret_cast<type>(HXbtree_get(bt, ptr));
}

template<typename type> static inline type
HXbtree_del(struct HXbtree *bt, const void *ptr)
{
	return reinterpret_cast<type>(HXbtree_del(bt, ptr));
}

template<typename type> static inline type HXdeque_pop(struct HXdeque *dq)
{
	return reinterpret_cast<type>(HXdeque_pop(dq));
}

template<typename type> static inline type HXdeque_shift(struct HXdeque *dq)
{
	return reinterpret_cast<type>(HXdeque_shift(dq));
}

template<typename type> static inline type
HXdeque_get(struct HXdeque *dq, const void *ptr)
{
	return reinterpret_cast<type>(HXdeque_get(dq, ptr));
}

template<typename type> static inline type
HXdeque_del(struct HXdeque_node *nd)
{
	return reinterpret_cast<type>(HXdeque_del(nd));
}

template<typename type> static inline type *
HXdeque_to_vec(struct HXdeque *dq, long *n)
{
	return reinterpret_cast<type *>(HXdeque_to_vec(dq, n));
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
	if(src == NULL)
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
