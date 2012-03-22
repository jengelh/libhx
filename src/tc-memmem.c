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
static const char filler_text[] =
	"Slhrdlu cringle tongle flonging blobbity bleep blingmangl";

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
	printf("Start=%p End=%p\n", filler_text,
	       filler_text + ARRAY_SIZE(filler_text));
	printf("%p\n", HX_memmem(filler_text, strlen(filler_text), "nangl", 5));
	printf("%p\n", HX_memmem(filler_text, strlen(filler_text), "angl", 4));
	printf("%p\n", HX_memmem(filler_text, strlen(filler_text), "ngl", 3));

	for (i = 0; i < 10; ++i) {
		printf("Search length %u...", i);
		clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
		HX_memmem(haystack, size, haystack + size - i, i);
		clock_gettime(CLOCK_THREAD_CPUTIME_ID, &stop);
		HX_timespec_sub(&delta, &stop, &start);
		printf(HX_TIMESPEC_FMT "\n", HX_TIMESPEC_EXP(&delta));
	}

	HX_exit();
	return EXIT_SUCCESS;
}
