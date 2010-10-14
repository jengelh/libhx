/*
 *	libHX/assorted/tofrac.c
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 1999 - 2010
 *
 *	This file is part of libHX. libHX is free software; you can
 *	redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2 or 3 of the License.
 */
/*
 *	Calculates a readable fraction (i.e. 1/3) from arg and puts the
 *	*numerator into num, the denominator into *denom. Since the fraction
 *	is found out by an iterative loop, you can specify the minimum value
 *	of the denominator in *num and the maximum value of the denominator
 *	into *denom before calling the function.
 *
 *	If a suitable fraction has been found (within the range of the
 *	minimum / maximum denominator, *num and *denom will be overwritten
 *	with the results and true is returned; false for no success.
 *
 *	You need to re-put your min/max denom values into *num and *denom
 *	then.
 */
#include <sys/types.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/* This simplistic version does not deal with negative numbers. */

static bool HX_tofrac(double arg, unsigned long *num, unsigned long *denom)
{
	size_t i, min_denom = *num, max_denom = *denom;
	char simple[32], rounded[32];
	double j;

	for (i = min_denom; i < max_denom; ++i) {
		j = arg * i;
		snprintf(simple, sizeof(simple), "%f", j);
		snprintf(rounded, sizeof(rounded), "%.0f", j);
		j = strtod(rounded, 0);
		if (strtod(simple, NULL) == j) {
			*num   = j;
			*denom = i;
			return true;
		}
	}
	return false;
}

int main(int argc, const char **argv)
{
	/* A high denominator @n gives high precision of the fraction */
	unsigned long d = 1, n = ULLONG_MAX;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s 3.141592\n", *argv);
		return EXIT_FAILURE;
	}

	if (!HX_tofrac(strtod(argv[1], NULL), &d, &n)) {
		fprintf(stderr, "Our algorithm was too weak :-)\n");
		return EXIT_FAILURE;
	}

	printf("%lu/%lu\n", d, n);
	return EXIT_SUCCESS;
}
