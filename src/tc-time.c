/*
 *	Copyright © Jan Engelhardt
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the WTF Public License version 2 or
 *	(at your option) any later version.
 */
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <libHX/defs.h>
#include <libHX/init.h>
#include <libHX/misc.h>
#include "internal.h"

static unsigned int sleep_amt = 1336670;

static void zadd(void)
{
	static const struct timespec a = {0, 999999999};
	struct timespec r;

	HX_timespec_add(&r, &a, &a);
	printf(HX_TIMESPEC_FMT " = 2 x " HX_TIMESPEC_FMT "\n",
	       HX_TIMESPEC_EXP(&r), HX_TIMESPEC_EXP(&a));
}

static void zsub(void)
{
	static const struct timespec a = {1, 0}, b = {0, 1};
//	HX_timespec_sub(&r, &a, &b);
//	printf(HX_TIMESPEC_FMT " = " HX_TIMESPEC_FMT " - " HX_TIMESPEC_FMT "\n",
}

static void zsleep(void)
{
	struct timeval  m_past, m_future, m_delta;
	struct timespec n_past, n_future, n_delta;

	printf("µsec sleep: %u\n", sleep_amt);

	clock_gettime(CLOCK_REALTIME, &n_past);
	gettimeofday(&m_past, NULL);
	usleep(sleep_amt);
	clock_gettime(CLOCK_REALTIME, &n_future);
	gettimeofday(&m_future, NULL);

	HX_timeval_sub(&m_delta, &m_future, &m_past);
	printf("µsec: " HX_TIMEVAL_FMT " -> " HX_TIMEVAL_FMT
	       " = " HX_TIMEVAL_FMT "\n",
	       HX_TIMEVAL_EXP(&m_past), HX_TIMEVAL_EXP(&m_future),
	       HX_TIMEVAL_EXP(&m_delta));

	HX_timespec_sub(&n_delta, &n_future, &n_past);
	printf("nsec: " HX_TIMESPEC_FMT " -> " HX_TIMESPEC_FMT
	       " = " HX_TIMESPEC_FMT "\n",
	       HX_TIMESPEC_EXP(&n_past), HX_TIMESPEC_EXP(&n_future),
	       HX_TIMESPEC_EXP(&n_delta));

	HX_timespec_sub(&n_delta, &n_past, &n_future);
	printf("ns-1: " HX_TIMESPEC_FMT " -> " HX_TIMESPEC_FMT
	       " = " HX_TIMESPEC_FMT "\n",
	       HX_TIMESPEC_EXP(&n_past), HX_TIMESPEC_EXP(&n_future),
	       HX_TIMESPEC_EXP(&n_delta));
}

int main(void)
{
	if (HX_init() <= 0)
		abort();

	//zact();
	zsleep();
	HX_exit();
	return EXIT_SUCCESS;
}
