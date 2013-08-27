/*
 *	Copyright Jan Engelhardt
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the WTF Public License version 2 or
 *	(at your option) any later version.
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <libHX/init.h>
#include <libHX/misc.h>
#include "internal.h"

int main(void)
{
	struct timespec past, now, delta;
	unsigned int i;

	if (HX_init() <= 0)
		abort();
	for (i = 0; i < 15; ++i) {
		printf("%d ", HX_irand(i, i));
		printf("%.1f ", HX_drand(i, i));
		printf("%08x, ", HX_irand(0, RAND_MAX));
	}
	printf("\n");

	clock_gettime(CLOCK_REALTIME, &past);
	for (i = 0; i < (1 << 25); ++i) {
		volatile unsigned int __attribute__((unused)) t =
			HX_irand(0, RAND_MAX);
	}
	clock_gettime(CLOCK_REALTIME, &now);
	HX_timespec_sub(&delta, &now, &past);
	printf("%% method: " HX_TIMESPEC_FMT " s\n", HX_TIMESPEC_EXP(&delta));

	clock_gettime(CLOCK_REALTIME, &past);
	for (i = 0; i < (1 << 25); ++i) {
		volatile unsigned int __attribute__((unused)) t =
			HX_irand(0, ~0U);
	}
	clock_gettime(CLOCK_REALTIME, &now);
	HX_timespec_sub(&delta, &now, &past);
	printf("/ method: " HX_TIMESPEC_FMT " s\n", HX_TIMESPEC_EXP(&delta));

	HX_exit();
	return EXIT_SUCCESS;
}
