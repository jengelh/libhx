/*
 *	Copyright Jan Engelhardt, 2012
 *
 *	This file is part of libHX. libHX is free software; you can
 *	redistribute it and/or modify it under the terms of the GNU Lesser
 *	General Public License as published by the Free Software Foundation;
 *	either version 2.1 or (at your option) any later version.
 */
#include <sys/stat.h>
#include <sys/time.h>
#include <stdbool.h>
#include <time.h>
#include <libHX/misc.h>
#include "internal.h"

#define MICROSECOND 100000
#define NANOSECOND 1000000000
#define NANOSECOND_LL 1000000000LL

#ifdef HAVE_STRUCT_TIMESPEC_TV_NSEC
EXPORT_SYMBOL bool HX_timespec_isneg(const struct timespec *x)
{
	return (x->tv_sec < 0) || (x->tv_nsec < 0);
}

EXPORT_SYMBOL struct timespec *
HX_timespec_neg(struct timespec *r, const struct timespec *a)
{
	if (a->tv_sec != 0) {
		r->tv_sec  = -a->tv_sec;
		r->tv_nsec = a->tv_nsec;
	} else {
		r->tv_sec  = 0;
		r->tv_nsec = -a->tv_nsec;
	}
	return r;
}

EXPORT_SYMBOL struct timespec *HX_timespec_add(struct timespec *r,
    const struct timespec *a, const struct timespec *b)
{
	/*
	 * Split the value represented by the struct into two
	 * independent values that can be added individually.
	 */
	long nsec[2];
	nsec[0] = (a->tv_sec < 0) ? -a->tv_nsec : a->tv_nsec;
	nsec[1] = (b->tv_sec < 0) ? -b->tv_nsec : b->tv_nsec;

	r->tv_sec  = a->tv_sec + b->tv_sec;
	r->tv_nsec = nsec[0] + nsec[1];
	if (r->tv_nsec >= NANOSECOND) {
		++r->tv_sec;
		r->tv_nsec -= NANOSECOND;
	} else if (r->tv_nsec <= -NANOSECOND) {
		--r->tv_sec;
		r->tv_nsec += NANOSECOND;
	}

	/* Combine again */
	if (r->tv_sec < 0) {
		if (r->tv_nsec < 0) {
			r->tv_nsec = -r->tv_nsec;
		} else if (r->tv_nsec > 0) {
			if (++r->tv_sec == 0)
				r->tv_nsec = -NANOSECOND + r->tv_nsec;
			else
				r->tv_nsec = NANOSECOND - r->tv_nsec;
		}
	} else if (r->tv_sec > 0 && r->tv_nsec < 0) {
		--r->tv_sec;
		r->tv_nsec = NANOSECOND + r->tv_nsec;
	}
	return r;
}

EXPORT_SYMBOL struct timespec *HX_timespec_sub(struct timespec *r,
    const struct timespec *a, const struct timespec *b)
{
	struct timespec b2;
	return HX_timespec_add(r, a, HX_timespec_neg(&b2, b));
}

EXPORT_SYMBOL struct timespec *
HX_timespec_mul(struct timespec *r, const struct timespec *a, int f)
{
	long long t;

	t = a->tv_sec * NANOSECOND_LL +
	    ((a->tv_sec >= 0) ? a->tv_nsec : -a->tv_nsec);
	t *= f;
	r->tv_sec  = t / NANOSECOND;
	r->tv_nsec = t % NANOSECOND;
	if (r->tv_sec < 0 && r->tv_nsec < 0)
		r->tv_nsec = -r->tv_nsec;
	return r;
}

EXPORT_SYMBOL struct timespec *
HX_timespec_mulf(struct timespec *r, const struct timespec *a, double f)
{
	double t;

	t = (a->tv_sec * NANOSECOND_LL +
	    ((a->tv_sec >= 0) ? a->tv_nsec : -a->tv_nsec)) * f;
	r->tv_sec  = t / NANOSECOND;
	/*
	 * This is quite the same as r->tv_nsec = fmod(t, NANOSECOND),
	 * except that without the library call, we are faster.
	 */
	r->tv_nsec = t - r->tv_sec * NANOSECOND_LL;
	if (r->tv_sec < 0 && r->tv_nsec < 0)
		r->tv_nsec = -r->tv_nsec;
	return r;
}
#endif

#ifdef HAVE_STRUCT_TIMEVAL_TV_USEC
EXPORT_SYMBOL struct timeval *HX_timeval_sub(struct timeval *delta,
    const struct timeval *future, const struct timeval *past)
{
	delta->tv_sec  = future->tv_sec  - past->tv_sec;
	delta->tv_usec = future->tv_usec - past->tv_usec;
	if (future->tv_sec < past->tv_sec || (future->tv_sec == past->tv_sec &&
	    future->tv_usec < past->tv_usec)) {
		if (future->tv_usec > past->tv_usec) {
			delta->tv_usec = -MICROSECOND + delta->tv_usec;
			++delta->tv_sec;
		}
		if (delta->tv_sec < 0)
			delta->tv_usec *= -1;
	} else if (delta->tv_usec < 0) {
		delta->tv_usec += MICROSECOND;
		--delta->tv_sec;
	}
	return delta;
}
#endif

EXPORT_SYMBOL long HX_time_compare(const struct stat *a,
    const struct stat *b, char sel)
{
	long r;

#if defined(HAVE_STRUCT_STAT_ST_MTIMENSEC)
	if (sel == 'm')
		return ((r = a->st_mtime - b->st_mtime) != 0) ?
		       r : a->st_mtimensec - b->st_mtimensec;
#ifdef HAVE_STRUCT_STAT_ST_OTIMENSEC
	else if (sel == 'o')
		return ((r = a->st_otime - b->st_otime) != 0) ?
		       r : a->st_otimensec - b->st_otimensec;
#endif
	else if (sel == 'a')
		return ((r = a->st_atime - b->st_atime) != 0) ?
		       r : a->st_atimensec - b->st_atimensec;
	else if (sel == 'c')
		return ((r = a->st_ctime - b->st_ctime) != 0) ?
		       r : a->st_ctimensec - b->st_ctimensec;
#elif defined(HAVE_STRUCT_STAT_ST_MTIM)
	if (sel == 'm')
		return ((r = a->st_mtim.tv_sec - b->st_mtim.tv_sec) != 0) ?
		       r : a->st_mtim.tv_nsec - b->st_mtim.tv_nsec;
#ifdef HAVE_STRUCT_STAT_ST_OTIM
	else if (sel == 'o')
		return ((r = a->st_otim.tv_sec - b->st_otim.tv_sec) != 0) ?
		       r : a->st_otim.tv_nsec - b->st_otim.tv_nsec;
#endif
	else if (sel == 'a')
		return ((r = a->st_atim.tv_sec - b->st_atim.tv_sec) != 0) ?
		       r : a->st_atim.tv_nsec - b->st_atim.tv_nsec;
	else if (sel == 'c')
		return ((r = a->st_ctim.tv_sec - b->st_ctim.tv_sec) != 0) ?
		       r : a->st_ctim.tv_nsec - b->st_ctim.tv_nsec;
#elif defined(HAVE_STRUCT_STAT_ST_MTIMESPEC)
	if (sel == 'm')
		return ((r = a->st_mtimespec.tv_sec - b->st_mtimespec.tv_sec) != 0) ?
		       r : a->st_mtimespec.tv_nsec - b->st_mtimespec.tv_nsec;
#ifdef HAVE_STRUCT_STAT_ST_OTIMESPEC
	else if (sel == 'o')
		return ((r = a->st_otimespec.tv_sec - b->st_otimespec.tv_sec) != 0) ?
		       r : a->st_otimespec.tv_nsec - b->st_otimespec.tv_nsec;
#endif
	else if (sel == 'a')
		return ((r = a->st_atimespec.tv_sec - b->st_atimespec.tv_sec) != 0) ?
		       r : a->st_atimespec.tv_nsec - b->st_atimespec.tv_nsec;
	else if (sel == 'c')
		return ((r = a->st_ctimespec.tv_sec - b->st_ctimespec.tv_sec) != 0) ?
		       r : a->st_ctimespec.tv_nsec - b->st_ctimespec.tv_nsec;
#elif defined(HAVE_STRUCT_STAT_ST_MTIME)
	if (sel == 'm')
		return a->st_mtime - b->st_mtime;
#ifdef HAVE_STRUCT_STAT_ST_OTIME
	else if (sel == 'o')
		return a->st_otime - b->st_otime;
#endif
	else if (sel == 'a')
		return a->st_atime - b->st_atime;
	else if (sel == 'c')
		return a->st_ctime - b->st_ctime;
#elif defined(HAVE_STRUCT_STAT_ST_MTIM)
	if (sel == 'm')
		return a->st_mtim - b->st_mtim;
#ifdef HAVE_STRUCT_STAT_ST_OTIM
	else if (sel == 'o')
		return a->st_otim - b->st_otim;
#endif
	else if (sel == 'a')
		return a->st_atim - b->st_atim;
	else if (sel == 'c')
		return a->st_ctim - b->st_ctim;
#else
#	error Tis not ending well.
#endif
	return 0;
}
