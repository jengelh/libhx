/*
 *	This program is in the Public Domain
 */
#ifdef __cplusplus
#	include <cstdio>
#	include <cstdlib>
#	include <ctime>
#else
#	include <stdio.h>
#	include <stdlib.h>
#	include <time.h>
#endif
#include <libHX/map.h>
#include <libHX/misc.h>
#include "internal.h"

enum {
	NUM_ENTRIES = 100000,
};

/**
 * tmap_rword - create random word
 * @dest:	char buffer
 * @length:	size of buffer
 */
static inline void tmap_rword(char *dest, unsigned int length)
{
	while (--length > 0)
		*dest++ = HX_irand('a', 'z' + 1);
	*dest = '\0';
}

static void tmap_add_speed(struct HXmap *map)
{
	char key[8], value[HXSIZEOF_Z32];
	struct timespec start, stop, delta;
	unsigned int i;

	key[sizeof(key)-1] = '\0';
	clock_gettime(CLOCK_REALTIME, &start);
	for (i = 0; i < NUM_ENTRIES; ++i) {
		tmap_rword(key, sizeof(key));
		snprintf(value, sizeof(value), "%u", i);
		HXmap_add(map, key, value);
	}
	HXmap_add(map, "fruit", "apple");
	clock_gettime(CLOCK_REALTIME, &stop);
	HX_diff_timespec(&delta, &stop, &start);
	printf("Raw add speed: %ld.%09ld\n",
	       static_cast(long, delta.tv_sec), delta.tv_nsec);
}

static void test_map(struct HXmap *map)
{
	tmap_add_speed(map);
	printf("fruit=%s\n",
	       static_cast(const char *, HXmap_get(map, "fruit")));
	HXmap_free(map);
}

int main(void)
{
	printf("* HXhashmap\n");
	test_map(HXhashmap_init(HXMAP_SCKEY | HXMAP_SCDATA));

	return EXIT_SUCCESS;
}
