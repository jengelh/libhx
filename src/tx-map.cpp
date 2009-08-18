/*
 *	This program is in the Public Domain
 */
#ifdef __cplusplus
#	include <cstdarg>
#	include <cstdio>
#	include <cstdlib>
#	include <ctime>
#else
#	include <stdarg.h>
#	include <stdio.h>
#	include <stdlib.h>
#	include <time.h>
#endif
#include <libHX/map.h>
#include <libHX/misc.h>
#include "internal.h"

static unsigned int tmap_indent = 0;

static inline void tmap_ipush(void)
{
	++tmap_indent;
}

static inline void tmap_ipop(void)
{
	if (tmap_indent > 0)
		--tmap_indent;
}

static void tmap_printf(const char *fmt, ...)
{
	unsigned int i;
	va_list args;

	for (i = 0; i < tmap_indent; ++i)
		printf("\t");
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}

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
	unsigned int i = 0;

	tmap_printf("Timing add operation\n");
	key[sizeof(key)-1] = '\0';
	clock_gettime(CLOCK_REALTIME, &start);
	do {
		tmap_rword(key, sizeof(key));
		snprintf(value, sizeof(value), "%u", i++);
		HXmap_add(map, key, value);
		clock_gettime(CLOCK_REALTIME, &stop);
		HX_diff_timespec(&delta, &stop, &start);
	} while (!(delta.tv_sec >= 1 || map->items >= 1000000));
	tmap_ipush();
	tmap_printf("%u elements in %ld.%09ld "
		"(plus time measurement overhead)\n",
		map->items, static_cast(long, delta.tv_sec), delta.tv_nsec);
	tmap_ipop();
}

static void tmap_add_rand(struct HXmap *map, unsigned int num)
{
	char key[8], value[HXSIZEOF_Z32];

	while (num-- > 0) {
		tmap_rword(key, sizeof(key));
		snprintf(value, sizeof(value), "%u", num);
		HXmap_add(map, key, value);
	}
}

static void tmap_trav(struct HXmap *map)
{
	const struct HXmap_node *node;
	unsigned int i = ~(~0U >> 1);
	char key[8], value[HXSIZEOF_Z32];
	void *iter;

	tmap_printf("Simple traversal:\n");
	tmap_ipush();
	iter = HXmap_travinit(map, 0);
	while ((node = HXmap_traverse(iter)) != NULL)
		tmap_printf("%s -> %s\n", node->skey, node->sdata);
	tmap_ipop();
	HXmap_travfree(iter);

	tmap_printf("Add modification during traversal:\n");
	tmap_ipush();
	iter = HXmap_travinit(map, 0);
	while ((node = HXmap_traverse(iter)) != NULL) {
		tmap_printf("%s -> %s\n", node->skey, node->sdata);
		tmap_rword(key, sizeof(key));
		snprintf(value, sizeof(value), "%u", i++);
		HXmap_add(map, key, value);
	}
	tmap_ipop();
	HXmap_travfree(iter);
}

static void tmap_del(struct HXmap *map, bool verbose)
{
	const struct HXmap_node *node;
	void *iter;

	tmap_printf("Deletion of %u elements, with traversal:\n", map->items);
	tmap_ipush();
	while (map->items != 0) {
		/* May need to reload traverser due to deletion */
		if (verbose)
			tmap_printf("Restarting traverser\n");
		if ((iter = HXmap_travinit(map, HXMAP_DTRAV)) == NULL)
			break;
		tmap_ipush();
		while ((node = HXmap_traverse(iter)) != NULL) {
			if (verbose)
				tmap_printf("Destroying {%s, %s}\n",
					node->skey, node->sdata);
			HXmap_del(map, node->key);
		}
		tmap_ipop();
		HXmap_travfree(iter);
	}
	tmap_ipop();
}

static void tmap_test(struct HXmap *(*create_map)(unsigned int),
    unsigned int flags)
{
	struct HXmap *map;

	map = (*create_map)(flags);
	tmap_add_speed(map);

	tmap_printf("Element retrieval:\n");
	HXmap_add(map, "fruit", "apple");
	tmap_printf("fruit=%s\n",
	       static_cast(const char *, HXmap_get(map, "fruit")));
	tmap_del(map, false);

	tmap_add_rand(map, 2);
	tmap_trav(map);
	tmap_del(map, true);
	HXmap_free(map);
}

static struct HXmap *tmap_create_hashmap(unsigned int flags)
{
	return HXhashmap_init(HXMAP_SCKEY | HXMAP_SCDATA | HXMAP_DTRAV);
}

static struct HXmap *tmap_create_rbtree(unsigned int flags)
{
	return HXrbtree_init(HXMAP_SCKEY | HXMAP_SCDATA | HXMAP_DTRAV);
}

int main(void)
{
	tmap_printf("* HXhashmap\n");
	tmap_test(tmap_create_hashmap, 0);

	tmap_printf("\n* RBtree\n");
	tmap_test(tmap_create_rbtree, 0);

	return EXIT_SUCCESS;
}
