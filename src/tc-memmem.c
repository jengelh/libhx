/*
 *	speed test HX_memmem
 *	written by Jan Engelhardt
 *	this program is released in the Public Domain
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <libHX/init.h>
#include <libHX/misc.h>
#include <libHX/string.h>

static unsigned int size = 1048576 * 128;

int main(void)
{
	unsigned int i;
	char *haystack;
	struct timespec start, stop, delta;

	if (HX_init() <= 0)
		abort();
	haystack = malloc(size);
	if (haystack == NULL)
		abort();
	memset(haystack, 'A', size);
	haystack[size-1] = haystack[size-2] = 'Z';
	printf("Init done\n");

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
	printf("Start: " HX_TIMESPEC_FMT "\n", HX_TIMESPEC_EXP(&start));
	HX_memmem(haystack, size, haystack + size - 1, 1);
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &stop);
	printf("Stop:  " HX_TIMESPEC_FMT "\n", HX_TIMESPEC_EXP(&stop));
	HX_timespec_sub(&delta, &stop, &start);
	printf("Time for full search: " HX_TIMESPEC_FMT "\n",
	       HX_TIMESPEC_EXP(&delta));

	HX_exit();
	return EXIT_SUCCESS;
}
