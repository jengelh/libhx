/*
 *	libHX/rand.c
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2003 - 2008
 *
 *	This file is part of libHX. libHX is free software; you can
 *	redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include "config.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef __unix__
#	include <unistd.h>
#endif
#ifdef HAVE_GETTIMEOFDAY
#	include <sys/time.h>
#endif
#include <libHX/misc.h>
#include "internal.h"

static unsigned int HXrand_obtain_seed(void)
{
	unsigned int s;

#if defined(HAVE_GETTIMEOFDAY)
	struct timeval tv;

	gettimeofday(&tv, NULL);
	s  = tv.tv_sec;
	s ^= tv.tv_usec << 16;
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

static __attribute__((constructor)) void HXrand_init(void)
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

EXPORT_SYMBOL int HX_rand(void)
{
	/*
	 * If there is an overly broken system, we may need to use
	 * alternate methods again (/dev/urandom?)
	 */
	return rand();
}

EXPORT_SYMBOL unsigned int HX_irand(unsigned int lo, unsigned int hi)
{
	unsigned int delta = hi - lo;

	if (delta <= RAND_MAX)
		return HX_rand() % delta + lo;
	else
		return static_cast(unsigned int,
		       static_cast(double, HX_rand()) * delta / RAND_MAX) + lo;
}
