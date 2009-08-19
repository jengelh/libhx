/*
 *	This program is in the Public Domain
 */
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <libHX/map.h>
#include <libHX/misc.h>
#include <libHX/string.h>
#include "internal.h"
#include "map_int.h"

union HXrbt {
	struct HXmap *map;
	struct HXrbtree *rbt;
};

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

static bool tmap_each_fn(const struct HXmap_node *node, void *arg)
{
	return true;
}

static void tmap_trav_speed(struct HXmap *map)
{
	struct timespec start, stop, delta;
	const struct HXmap_node *node;
	void *iter;

	tmap_printf("Timing traversal\n");
	tmap_ipush();
	iter = HXmap_travinit(map, 0);
	clock_gettime(CLOCK_REALTIME, &start);
	while ((node = HXmap_traverse(iter)) != NULL)
		;
	clock_gettime(CLOCK_REALTIME, &stop);
	HX_diff_timespec(&delta, &stop, &start);
	HXmap_travfree(iter);
	tmap_printf("Open traversal of %u nodes: %ld.%09lds\n",
		map->items, static_cast(long, delta.tv_sec), delta.tv_nsec);

	clock_gettime(CLOCK_REALTIME, &start);
	HXmap_qfe(map, tmap_each_fn, NULL);
	clock_gettime(CLOCK_REALTIME, &stop);
	HX_diff_timespec(&delta, &stop, &start);
	tmap_printf("QFE traversal of %u nodes: %ld.%09lds\n",
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

static void tmap_flat(const struct HXmap *map)
{
	struct HXmap_node *nodes;
	unsigned int i;

	tmap_printf("Retrieving flattened list of %u elements:\n", map->items);
	tmap_ipush();
	nodes = HXmap_keysvalues(map);
	if (nodes == NULL) {
		perror("HXmap_keysvalues");
		abort();
	}
	for (i = 0; i < map->items; ++i)
		tmap_printf("%u. %s -> %s\n", i, nodes[i].key, nodes[i].data);
	tmap_ipop();
	free(nodes);
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
	tmap_trav_speed(map);

	tmap_printf("Element retrieval:\n");
	HXmap_add(map, "fruit", "apple");
	tmap_printf("fruit=%s\n",
	       static_cast(const char *, HXmap_get(map, "fruit")));
	tmap_del(map, false);

	tmap_add_rand(map, 2);
	tmap_flat(map);
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

static int tmap_strtolcmp(const void *a, const void *b, size_t z)
{
	return strtol(static_cast(const char *, a), NULL, 0) -
	       strtol(static_cast(const char *, b), NULL, 0);
}

static const struct HXmap_ops tmap_nstr_ops = {
	.k_compare = tmap_strtolcmp,
};

static void __rbt_walk_tree(const struct HXrbtree_node *node,
    char *buf, size_t s)
{
	bool has_children = node->sub[0] != NULL || node->sub[1] != NULL;
	HX_strlcat(buf, node->skey, s);

	if (node->color == RBT_BLACK)
		HX_strlcat(buf, "%b", s);
	if (has_children)
		HX_strlcat(buf, "(" /* ) */, s);
	if (node->sub[0] != NULL)
		__rbt_walk_tree(node->sub[0], buf, s);
	if (node->sub[1] != NULL) {
		HX_strlcat(buf, ",", s);
		__rbt_walk_tree(node->sub[1], buf, s);
	}
	if (has_children)
		HX_strlcat(buf, /* ( */ ")", s);
}

/**
 * rbt_walk_tree - walk the tree and provide a string suitable for texitree
 * @node:	node of an rbtree to start diving at
 * @buf:	buffer for texitree representation
 * @size:	size for @buf
 */
static void rbt_walk_tree(const struct HXrbtree_node *node,
    char *buf, size_t size)
{
	*buf = '\0';
	__rbt_walk_tree(node, buf, size);
}

/**
 * rbt_check - compare two texitree representations or warn
 * @result:	result from previous operations
 * @expected:	what we think should have happened
 */
static int rbt_check(const char *result, const char *expected)
{
	int ret = strcmp(result, expected);
	tmap_ipush();
	tmap_printf("Expected: %s\n", expected);
	tmap_printf("  Result: %s\n", result);
	if (ret != 0) {
		tmap_ipush();
		tmap_printf("...failed\n");
		tmap_ipop();
	}
	tmap_ipop();
	return ret;
}

/**
 * rbt_new_perfect_tree - generate a perfect binary tree
 * @height:	height of the desired tree
 * @mult:	multiplicator for node numbers
 *
 * Produces a tree of desired height with exactly 2^height-1 nodes.
 */
static struct HXmap *rbt_new_perfect_tree(unsigned int height,
    unsigned int mult)
{
	unsigned int right = 1 << height;
	unsigned int incr  = right;
	unsigned int left  = incr / 2;
	unsigned int y, x;
	union HXrbt u;
	char buf[HXSIZEOF_Z32];

	u.map = HXrbtree_init4(HXMAP_SKEY | HXMAP_CKEY, &tmap_nstr_ops, 0, 0);
	if (u.map == NULL)
		abort();

	for (y = 0; y < height; ++y) {
		for (x = left; x < right; x += incr) {
			snprintf(buf, sizeof(buf), "%u", x * mult);
			HXmap_add(u.map, buf, NULL);
		}
		incr /= 2;
		left /= 2;
	}

	return u.map;
}

static unsigned int rbt_tree_height(const struct HXrbtree_node *node)
{
	unsigned int a = 1, b = 1;
	if (node->sub[0] != NULL)
		a += rbt_tree_height(node->sub[0]);
	if (node->sub[1] != NULL)
		b += rbt_tree_height(node->sub[1]);
	return (a > b) ? a : b;
}

static void rbt_height_check(const struct HXrbtree *tree)
{
	double min, max, avg;
	min = log(tree->super.items + 1) / log(2);
	max = 2 * log(tree->super.items + 1) / log(2);
	avg = log((pow(2, min) + pow(2, max)) / 2) / log(2);
	tmap_ipush();
	tmap_printf("Item count: %u\n", tree->super.items);
	tmap_printf("Minimum height: %f\n", min);
	tmap_printf("Average height: %f\n", avg);
	tmap_printf("Maximum height: %f\n", max);
	tmap_printf("Current height: %u\n", rbt_tree_height(tree->root));
	tmap_ipop();
}

/**
 * tmap_rbt_test_1 - basic rbt node layout tests
 */
static void tmap_rbt_test_1(void)
{
	union HXrbt u;
	char buf[80];

	tmap_printf("RBT test 1A: Creating tree with 7 nodes (height 3)\n");
	u.map = rbt_new_perfect_tree(3, 2);

	tmap_printf("RBT test 1B: Manual traverse\n");
	rbt_walk_tree(u.rbt->root, buf, sizeof(buf));

	tmap_printf("RBT test 1C: Check for correct positions and colors\n");
	rbt_check(buf, "8%b(4%b(2,6),12%b(10,14))");

	/*        8
	 *      /     \
	 *    4         12
	 *   / \       /  \
	 *  2   6    10    14
	 *          /
	 *         9
	 */
	tmap_printf("RBT test 1D: Node insertion and test for positions/colors\n");
	HXmap_add(u.map, "9", NULL);
	rbt_walk_tree(u.rbt->root, buf, sizeof(buf));
	rbt_check(buf, "8%b(4%b(2,6),12(10%b(9),14%b))");

	tmap_printf("RBT test 1E: Height check\n");
	rbt_height_check(u.rbt);

	tmap_printf("RBT test 1G: Node deletion\n");
	HXmap_del(u.map, "8");
	rbt_walk_tree(u.rbt->root, buf, sizeof(buf));
	rbt_check(buf, "9%b(4%b(2,6),12(10%b,14%b))");

	/*        9       (8 replaced by its in-order successor 9)
	 *      /    \
	 *    4        12
	 *   / \      /  \
	 *  2   6   10    14
	 */
}

int main(void)
{
	tmap_printf("* HXhashmap\n");
	tmap_test(tmap_create_hashmap, 0);

	tmap_printf("\n* RBtree\n");
	tmap_test(tmap_create_rbtree, 0);
	tmap_rbt_test_1();

	return EXIT_SUCCESS;
}
