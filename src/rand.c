/*
 *	Random numbers
 *	Copyright Jan Engelhardt, 2003-2008
 *
 *	This file is part of libHX. libHX is free software; you can
 *	redistribute it and/or modify it under the terms of the GNU Lesser
 *	General Public License as published by the Free Software Foundation;
 *	either version 2.1 or (at your option) any later version.
 */
#include "config.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef __unix__
#	include <unistd.h>
#endif
#ifdef _WIN32
#	include <process.h>
#endif
#include <libHX/init.h>
#include <libHX/misc.h>
#include "internal.h"

static unsigned int HXrand_obtain_seed(void)
{
	unsigned int s;

#if defined(HAVE_CLOCK_GETTIME)
	struct timespec tv;

	clock_gettime(CLOCK_REALTIME, &tv);
	s  = tv.tv_sec;
	s ^= ~tv.tv_nsec;
	clock_gettime(CLOCK_MONOTONIC, &tv);
	s ^= tv.tv_sec;
	s ^= ~tv.tv_nsec;
#else
	s = time(NULL);
#endif
#ifdef HAVE_GETPID
	s ^= getpid() << 9;
#endif
#ifdef HAVE_GETPPID
	s ^= getppid() << 1;
#endif
#ifdef HAVE_GETEUID
	s ^= geteuid() << 13;
#endif
#ifdef HAVE_GETEGID
	s ^= getegid() << 5;
#endif
	return s;
}

static void HXrand_init(void)
{
	unsigned int seed;
	int fd, ret = 0;

	if ((fd = open("/dev/urandom", O_RDONLY | O_BINARY)) >= 0) {
		ret = read(fd, &seed, sizeof(seed));
		close(fd);
	}
	if (ret != sizeof(seed))
		seed = HXrand_obtain_seed();
	srand(seed);
}

static pthread_mutex_t HX_init_lock = PTHREAD_MUTEX_INITIALIZER;
static unsigned long HX_use_count;

EXPORT_SYMBOL int HX_init(void)
{
	pthread_mutex_lock(&HX_init_lock);
	if (HX_use_count == 0)
		HXrand_init();
	++HX_use_count;
	pthread_mutex_unlock(&HX_init_lock);
	return 1;
}

EXPORT_SYMBOL void HX_exit(void)
{
	pthread_mutex_lock(&HX_init_lock);
	if (HX_use_count == 0)
		fprintf(stderr, "%s: reference count is already zero!\n", __func__);
	else
		--HX_use_count;
	pthread_mutex_unlock(&HX_init_lock);
}

EXPORT_SYMBOL int HX_rand(void)
{
	/*
	 * If there is an overly broken system, we may need to use
	 * alternate methods again (/dev/urandom?)
	 */
	return rand();
}

EXPORT_SYMBOL double HX_drand(double lo, double hi)
{
	double delta = hi - lo;

	return static_cast(double, rand()) * delta / RAND_MAX + lo;
}

EXPORT_SYMBOL unsigned int HX_irand(unsigned int lo, unsigned int hi)
{
	unsigned int delta = hi - lo;

	if (delta == 0)
		return lo;
	else if (delta <= RAND_MAX)
		return rand() % delta + lo;
	else
		return static_cast(unsigned int,
		       static_cast(double, rand()) * delta / RAND_MAX) + lo;
}
