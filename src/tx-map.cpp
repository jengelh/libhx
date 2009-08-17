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
	NUM_ENTRIES = 25,
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

static void tmap_trav(struct HXmap *map)
{
	const struct HXmap_node *node;
	unsigned int i = (NUM_ENTRIES + 999) / 1000 * 1000;
	char key[8], value[HXSIZEOF_Z32];
	void *iter;

	printf("Simple traversal:\n");
	iter = HXmap_travinit(map);
	while ((node = HXmap_traverse(iter)) != NULL)
		printf("%s -> %s\n", node->skey, node->sdata);
	HXmap_travfree(iter);

	printf("Modification during traversal:\n");
	iter = HXmap_travinit(map);
	while ((node = HXmap_traverse(iter)) != NULL) {
		printf("%s -> %s\n", node->skey, node->sdata);
		tmap_rword(key, sizeof(key));
		snprintf(value, sizeof(value), "%u", i++);
		HXmap_add(map, key, value);
	}
	HXmap_travfree(iter);
}

static void tmap_del(struct HXmap *map)
{
	const struct HXmap_node *node;
	void *iter;

	printf("Deletion of %u elements:\n", map->items);
	while (map->items != 0) {
		/* May need to reload traverser due to deletion */
		printf("Restarting traverser\n");
		if ((iter = HXmap_travinit(map)) == NULL)
			break;
		while ((node = HXmap_traverse(iter)) != NULL) {
			printf("Destroying {%s, %s}\n",
			       node->skey, node->sdata);
			HXmap_del(map, node->key);
		}
		HXmap_travfree(iter);
	}
}

static void test_map(struct HXmap *map)
{
	tmap_add_speed(map);
	printf("fruit=%s\n",
	       static_cast(const char *, HXmap_get(map, "fruit")));
	tmap_trav(map);
	tmap_del(map);
	HXmap_free(map);
}

int main(void)
{
	printf("* HXhashmap\n");
	test_map(HXhashmap_init(HXMAP_SCKEY | HXMAP_SCDATA));
	printf("* RBtree\n");
	test_map(HXrbtree_init(HXMAP_SCKEY | HXMAP_SCDATA));

	return EXIT_SUCCESS;
}
