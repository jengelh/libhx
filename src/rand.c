/*
 *	libHX/rand.c
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2003 - 2008
 *
 *	This file is part of libHX. libHX is free software; you can
 *	redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef __unix__
#	include <unistd.h>
#endif
#include <libHX/misc.h>
#include "internal.h"

static int rand_fd = -1;

static __attribute__((constructor)) void HXrand_init(void)
{
	int fd;

	if (rand_fd == -1)
		if ((fd = open("/dev/urandom", O_RDONLY | O_BINARY)) >= 0)
			rand_fd = fd;

#ifdef __unix__
	srand(time(NULL) ^ ((getpid() + getppid() + geteuid() + getegid()) << 5));
#else
	srand(time(NULL));
#endif
}

static __attribute__((destructor)) void HXrand_deinit(void)
{
	close(rand_fd);
	rand_fd = -1;
}

EXPORT_SYMBOL int HX_rand(void)
{
	int n;
	if (rand_fd < 0 || read(rand_fd, &n, sizeof(n)) != sizeof(n))
		return rand();
	return (n >= 0) ? n : -n;
}

EXPORT_SYMBOL unsigned int HX_irand(unsigned int lo, unsigned int hi)
{
	return static_cast(unsigned int, static_cast(double, HX_rand()) *
	       (hi - lo) / RAND_MAX) + lo;
}
