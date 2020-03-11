/*
 *	Additional runtime checks
 *	Copyright Jan Engelhardt, 2011
 *
 *	This file is part of libHX. libHX is free software; you can
 *	redistribute it and/or modify it under the terms of the GNU Lesser
 *	General Public License as published by the Free Software Foundation;
 *	either version 2.1 or (at your option) any later version.
 *
 *	libHX_rtcheck.so is a library supposed to be used together with the
 *	LD_PRELOAD environment variable to dynamically add extra checks.
 */
#define _GNU_SOURCE 1
#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#ifdef RTLD_NEXT

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <libHX.h>
#include "internal.h"

#define call_next(f) \
	((__typeof__(f) *)dlsym(RTLD_NEXT, #f))

#define stub_head(f, args, invoke) \
	EXPORT_SYMBOL __typeof__(f invoke) f args \
	{ \
		if (HXrefchk_count <= 0) \
			fprintf(stderr, "%s: HX_init has not been called!\n", \
			        __func__);

#define stub_tail(f, params) \
		return call_next(f) params; \
	}

#define stub(f, args, invoke, params) \
	stub_head(f, args, invoke) \
	stub_tail(f, params)

#define stubv(f, args, params) \
	EXPORT_SYMBOL void f args \
	{ \
		if (HXrefchk_count <= 0) \
			fprintf(stderr, "%s: HX_init has not been called!\n", \
			        __func__); \
		call_next(f) params; \
	}

#define stub0(f)        stub(f, (void), (), ())
#define stub1(f, args)  stub(f, args, (0), (a))
#define stub1v(f, args) stubv(f, args, (a))
#define stub2(f, args)  stub(f, args, (0, 0), (a, b))
#define stub2v(f, args) stubv(f, args, (a, b))
#define stub3(f, args)  stub(f, args, (0, 0, 0), (a, b, c))
#define stub3v(f, args) stubv(f, args, (a, b, c))
#define stub4(f, args)  stub(f, args, (0, 0, 0, 0), (a, b, c, d))
#define stub5(f, args)  stub(f, args, (0, 0, 0, 0, 0), (a, b, c, d, e))

static pthread_mutex_t HXrefchk_lock = PTHREAD_MUTEX_INITIALIZER;
static unsigned long HXrefchk_count;

EXPORT_SYMBOL int HX_init(void)
{
	/*
	 * The real HX_init has its own reference count check, but that
	 * variable is not exported that we could test it, so the counter needs
	 * to be replicated here.
	 */
	pthread_mutex_lock(&HXrefchk_lock);
	if (HXrefchk_count == 0) {
		printf("# " PACKAGE_NAME " " PACKAGE_VERSION
		       " runtime checker active\n");
		call_next(HX_init)();
	}
	++HXrefchk_count;
	pthread_mutex_unlock(&HXrefchk_lock);
	return 1;
}

EXPORT_SYMBOL void HX_exit(void)
{
	pthread_mutex_lock(&HXrefchk_lock);
	if (HXrefchk_count == 0)
		fprintf(stderr, "%s: reference count is already zero!\n", __func__);
	else
		--HXrefchk_count;
	pthread_mutex_unlock(&HXrefchk_lock);
	call_next(HX_exit)();
}

/* deque.h */
stub0(HXdeque_init);
stub2(HXdeque_push, (struct HXdeque *a, const void *b));
stub2(HXdeque_unshift, (struct HXdeque *a, const void *b));
stub1(HXdeque_pop, (struct HXdeque *a));
stub1(HXdeque_shift, (struct HXdeque *a));
stub2(HXdeque_move, (struct HXdeque_node *a, struct HXdeque_node *b));
stub2(HXdeque_find, (struct HXdeque *a, const void *b));
stub2(HXdeque_get, (struct HXdeque *a, const void *b));
stub1(HXdeque_del, (struct HXdeque_node *a));
stub1v(HXdeque_free, (struct HXdeque *a));
stub2v(HXdeque_genocide2, (struct HXdeque *a, void (*b)(void *)));
stub2(HXdeque_to_vec, (const struct HXdeque *a, unsigned int *b));

/* io.h */
stub1(HXdir_open, (const char *a));
stub1(HXdir_read, (struct HXdir *a));
stub1v(HXdir_close, (struct HXdir *a));
/* HX_copy_dir: has varargs */
/* HX_copy_file: has varargs */
stub2(HX_mkdir, (const char *a, unsigned int b));
stub2(HX_readlink, (hxmc_t **a, const char *b));
stub3(HX_realpath, (hxmc_t **a, const char *b, unsigned int c));
stub1(HX_rrmdir, (const char *a));

stub3(HXio_fullread, (int a, void *b, size_t c));
stub3(HXio_fullwrite, (int a, const void *b, size_t c));

/* map.h */
stub2(HXmap_init, (enum HXmap_type a, unsigned int b));
stub5(HXmap_init5, (enum HXmap_type a, unsigned int b,
	const struct HXmap_ops *c, size_t d, size_t e));
stub3(HXmap_add, (struct HXmap *a, const void *b, const void *c));
stub2(HXmap_find, (const struct HXmap *a, const void *b));
stub2(HXmap_get, (const struct HXmap *a, const void *b));
stub2(HXmap_del, (struct HXmap *a, const void *b));
stub1(HXmap_keysvalues, (const struct HXmap *a));
stub2(HXmap_travinit, (const struct HXmap *a, unsigned int b));
stub1(HXmap_traverse, (struct HXmap_trav *a));
stub1v(HXmap_travfree, (struct HXmap_trav *a));
stub3v(HXmap_qfe, (const struct HXmap *a,
	bool (*b)(const struct HXmap_node *, void *), void *c));
stub1v(HXmap_free, (struct HXmap *a));

/* misc.h */
stub1(HX_dlopen, (const char *a));
stub2(HX_dlsym, (void *a, const char *b));
stub1v(HX_dlclose, (void *a));
stub0(HX_dlerror);

stub1(HX_ffs, (unsigned long a));
stub1(HX_fls, (unsigned long a));
stub3(HX_hexdump, (FILE *a, const void *b, unsigned int c));
stub3(HX_timespec_add, (struct timespec *a, const struct timespec *b,
	const struct timespec *c));
stub1(HX_timespec_isneg, (const struct timespec *a));
stub3(HX_timespec_mul, (struct timespec *a, const struct timespec *b, int c));
stub3(HX_timespec_mulf, (struct timespec *a, const struct timespec *b,
	double c));
stub2(HX_timespec_neg, (struct timespec *a, const struct timespec *b));
stub3(HX_timespec_sub, (struct timespec *a, const struct timespec *b,
	const struct timespec *c));
stub3(HX_timeval_sub, (struct timeval *a, const struct timeval *b,
	const struct timeval *c));
stub3(HX_time_compare, (const struct stat *a, const struct stat *b, char c));
stub1v(HX_zvecfree, (char **a));

stub0(HX_rand);
stub2(HX_irand, (unsigned int a, unsigned int b));
stub2(HX_drand, (double a, double b));

/* option.h */
stub0(HXformat_init);
stub1v(HXformat_free, (struct HXformat_map *a));
stub4(HXformat_add, (struct HXformat_map *a, const char *b, const void *c,
	unsigned int d));
stub3(HXformat_aprintf, (const struct HXformat_map *a, hxmc_t **b,
	const char *c));
stub4(HXformat_sprintf, (const struct HXformat_map *a, char *b, size_t c,
	const char *d));
stub3(HXformat_fprintf, (const struct HXformat_map *a, FILE *b,
	const char *c));

stub4(HX_getopt, (const struct HXoption *a, int *b, const char ***c,
	unsigned int d));
/* HX_getopt_help: not really public */
/* HX_getopt_help_cb: not really public */
/* HX_getopt_usage: not really public */
/* HX_getopt_usage_cb: not really public */
stub2(HX_shconfig, (const char *a, const struct HXoption *b));
stub1(HX_shconfig_map, (const char *a));
stub4(HX_shconfig_pv, (const char **a, const char *b, const struct HXoption *c,
	unsigned int d));
stub1v(HX_shconfig_free, (const struct HXoption *a));

/* proc.h */
stub2(HXproc_run_async, (const char *const *a, struct HXproc *b));
stub2(HXproc_run_sync, (const char *const *a, unsigned int b));
stub1(HXproc_wait, (struct HXproc *a));

/* string.h */
static __inline__ struct memcont *HXmc_base(const hxmc_t *p)
{
	return containerof(p, struct memcont, data);
}

static __inline__ void HXmc_check(const char *func, const void *cv)
{
	const struct memcont *c;

	if (cv == NULL)
		return;
	c = HXmc_base(cv);
	if (c->id != HXMC_IDENT)
		fprintf(stderr, "%s: %p is not a HXmc object!\n", func, cv);
}

stub1(HXmc_strinit, (const char *a));
stub2(HXmc_meminit, (const void *a, size_t b));

stub_head(HXmc_strcpy, (hxmc_t **a, const char *b), (0, 0))
{
	if (*a != NULL)
		HXmc_check(__func__, *a);
}
stub_tail(HXmc_strcpy, (a, b))

stub_head(HXmc_memcpy, (hxmc_t **a, const void *b, size_t c), (0, 0, 0))
{
	if (*a != NULL)
		HXmc_check(__func__, *a);
}
stub_tail(HXmc_memcpy, (a, b, c))

stub_head(HXmc_length, (const hxmc_t *a), (0))
{
	if (a != NULL)
		HXmc_check(__func__, a);
}
stub_tail(HXmc_length, (a))

stub2(HXmc_setlen, (hxmc_t **a, size_t b));
stub2(HXmc_trunc, (hxmc_t **a, size_t b));
stub2(HXmc_strcat, (hxmc_t **a, const char *b));
stub3(HXmc_memcat, (hxmc_t **a, const void *b, size_t c));
stub2(HXmc_strpcat, (hxmc_t **a, const char *b));
stub3(HXmc_mempcat, (hxmc_t **a, const void *b, size_t c));
stub3(HXmc_strins, (hxmc_t **a, size_t b, const char *c));
stub4(HXmc_memins, (hxmc_t **a, size_t b, const void *c, size_t d));
stub3(HXmc_memdel, (hxmc_t *a, size_t b, size_t c));
stub1v(HXmc_free, (hxmc_t *a));
stub1v(HXmc_zvecfree, (hxmc_t **a));

stub1(HX_basename, (const char *a));
stub1(HX_basename_exact, (const char *a));
stub1(HX_chomp, (char *a));
stub1(HX_dirname, (const char *a));
stub2(HX_getl, (hxmc_t **a, FILE *b));
stub4(HX_memmem, (const void *a, size_t b, const void *c, size_t d));
stub4(HX_split, (const char *a, const char *b, int *c, int d));
stub4(HX_split_inplace, (char *a, const char *b, int *c, int d));
stub4(HX_split_fixed, (char *a, const char *b, int c, char **d));
stub1(HX_stpltrim, (const char *a));
stub1(HX_stprtrim, (char *a));
stub3(HX_strbchr, (const char *a, const char *b, char c));
stub2(HX_strclone, (char **a, const char *b));
stub1(HX_strlower, (char *a));
stub1(HX_strltrim, (char *a));
stub3(HX_strmid, (const char *a, long b, long c));
stub2(HX_strndup, (const char *a, size_t b));
stub2(HX_strnlen, (const char *a, size_t b));
stub3(HX_strquote, (const char *a, unsigned int b, char **c));
stub2(HX_strrcspn, (const char *a, const char *b));
stub1(HX_strrev, (char *a));
stub1(HX_strrtrim, (char *a));
stub2(HX_strsep, (char **a, const char *b));
stub2(HX_strsep2, (char **a, const char *b));
stub1(HX_strupper, (char *a));

#endif /* RTLD_NEXT */
#endif /* HAVE_DLFCN_H */
