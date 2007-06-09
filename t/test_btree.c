#include <sys/time.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX.h>

enum {
	NODE_RED = 0,
	NODE_BLACK,
	Z_32 = sizeof("4294967296"),
};

static struct HXbtree *generate_fixed_tree(unsigned int, ...);
static struct HXbtree *generate_perfect_tree(unsigned int, unsigned int);
static void height_check(const struct HXbtree *);
static int sbc_strcmp(const char *, const char *);
static int strtolcmp(const void *, const void *);
static inline void timer_start(void);
static inline void timer_end(void);
static unsigned int tree_height(const struct HXbtree_node *);
static void __walk_tree(const struct HXbtree_node *, char *, size_t);
static void walk_tree(const struct HXbtree_node *, char *, size_t);

static const char *const Color[] = {"RED", "BLACK"};
static struct HXbtree *btree;
static struct timeval tv_start;

//-----------------------------------------------------------------------------
static void test_1(void)
{
	struct HXbtree_node *node;
	struct HXbtrav *trav;
	char buf[80];

	printf("Test 1A: Creating tree with 7 elements (hg 3)\n");
	btree = generate_perfect_tree(3, 2);

	printf("Test 1B: Manual traversion\n");
	walk_tree(btree->root, buf, sizeof(buf));

	printf("Test 1C: Check for correct positions and colors\n");
	sbc_strcmp(buf, "8%b(4%b(2,6),12%b(10,14))");

	/*        8
	 *      /     \
	 *    4         12
	 *   / \       /  \
	 *  2   6    10    14
	 *          /
	 *         9
	 */
	printf("Test 1D: Node insertion and test for positions/colors\n");
	HXbtree_add(btree, "9");
	walk_tree(btree->root, buf, sizeof(buf));
	sbc_strcmp(buf, "8%b(4%b(2,6),12(10%b(9),14%b))");

	printf("Test 1E: Height check\n");
	height_check(btree);

	printf("Test 1F: Standard traverse\n");
	trav = HXbtrav_init(btree);
	while((node = HXbtraverse(trav)) != NULL)
		printf("\t" "key: %s (%s)\n", (const char *)node->key,
		       Color[node->color]);
	HXbtrav_free(trav);

	printf("Test 1G: Node deletion\n");
	HXbtree_del(btree, "8");
	walk_tree(btree->root, buf, sizeof(buf));
	sbc_strcmp(buf, "9%b(4%b(2,6),12(10%b,14%b))");

	/*        9       (8 replaced by its in-order successor 9)
	 *      /    \
	 *    4        12
	 *   / \      /  \
	 *  2   6   10    14
	 */
	return;
}

static void test_2(void)
{
	const struct HXbtree_node *node;
	struct HXbtrav *trav;
	char buf[80];

	printf("Test 2A: Traverse with B-tree change\n");
	trav = HXbtrav_init(btree);
	while((node = HXbtraverse(trav)) != NULL) {
		walk_tree(btree->root, buf, sizeof(buf));
		printf("\t" "tree: %s\n", buf);
		printf("\t" " key: %s (%s)\n",
		       (const char *)node->key, Color[node->color]);
		if(strcmp(node->key, "4") == 0) {
			printf("\t" "Deleting [current] node \"4\"\n");
			HXbtree_del(btree, "4");
		} else if(strcmp(node->key, "12") == 0) {
			printf("\t" "Deleting [next] node \"14\"\n");
			HXbtree_del(btree, "14");
		}
	}

	HXbtrav_free(trav);
	/*        9
	 *      /    \
	 *    6        12
	 *   /        /
	 *  2       10
	 */

	printf("Test 2B: Traverse with B-tree destruction\n");
	trav = HXbtrav_init(btree);
	while((node = HXbtraverse(trav)) != NULL) {
		printf("\t" "About to delete \"%s\"\n",
		       (const char *)node->key);
		HXbtree_del(btree, node->key);
	}

	HXbtrav_free(trav);
	printf("\t" "Elements: %u (should be 0)\n", btree->items);
	printf("\t" "Root: %p (should be NULL)\n", btree->root);

	printf("Test 2C: Traversing empty tree\n");
	trav = HXbtrav_init(btree);
	while(HXbtraverse(trav) != NULL)
		printf("\t" "...failed\n");

	HXbtrav_free(trav);
	HXbtree_free(btree);
	printf("Test 2D: Pick a memory debugger, check the leaks now.\n");
	return;
}

static void test_3(void)
{
	const struct HXbtree_node *node;
	struct HXbtrav *trav;

	printf("Test 3: Creating tree with 63 elements (hg 6), "
	       "testing traverser path pickup code\n");
	btree = generate_perfect_tree(6, 1);
	trav = HXbtrav_init(btree);

	printf("\t");
	while ((node = HXbtraverse(trav)) != NULL) {
		printf("%s", (const char *)node->key);
		if (strcmp(node->key, "21") == 0) {
			HXbtree_del(btree, "21");
			printf("*");
		}
		printf(", ");
	}

	HXbtrav_free(trav);
	HXbtree_free(btree);
	printf("done\n");
	return;
}

static void test_4(void)
{
	unsigned int n;
	int hg, i;

	/*
	 * See how the tree expands under rather bad conditions
	 * (it is not the worst case it seems, however).
	 */
	printf("Test 4A: Tree height expansion check\n");
	btree = HXbtree_init(HXBT_ICMP);

	timer_start();
	for(n = 1; n != 0; ++n) {
		HXbtree_add(btree, (const void *)n);
		if((n & 0xFFFFF) != 0)
			continue;
		hg = tree_height(btree->root);
		printf("\t%u objects, height %d\n", btree->items, hg);
		if(hg == 47)
			break;
	}
	timer_end();

	/* Once used to see if alignment could be optimized */
	printf("Test 4B: Lookup speed\n");
	for(i = 0; i < 5; ++i) {
		timer_start();
		for(n = btree->items; n >= 1; --n)
			HXbtree_find(btree, (const void *)n);
		timer_end();
	}

	HXbtree_free(btree);
	return;
}

static void test_5(void)
{
	/*
	 *         8
	 *       /   \
	 *     4      12
	 *    / \    /  \
	 *   2   6 10    14
	 */
	char buf[80];

	printf("Test 5: Checking simple deletion (replace) logic\n");
	btree = generate_perfect_tree(3, 2);
	walk_tree(btree->root, buf, sizeof(buf));
	printf("\t%s\n", buf);
	HXbtree_del(btree, HX_rand() % 2 ? "2" : "6");
	walk_tree(btree->root, buf, sizeof(buf));
	printf("\t%s\n", buf);
	HXbtree_del(btree, "4");
	walk_tree(btree->root, buf, sizeof(buf));
	printf("\t%s\n", buf);
	HXbtree_free(btree);
	return;
}

static unsigned int test_6b(const struct HXbtree_node *node,
    unsigned int black_height, unsigned int counted_height)
{
	unsigned int ret = 1;

	if (node->color == NODE_BLACK)
		++counted_height;

	if (node->sub[0] == NULL && node->sub[1] == NULL &&
	    black_height != counted_height)
		/* Black height violated */
		return 0;

	if (node->sub[0] != NULL)
		ret &= test_6b(node->sub[0], black_height, counted_height);
	if (node->sub[1] != NULL)
		ret &= test_6b(node->sub[1], black_height, counted_height);

	return ret;
}

static void test_6(void)
{
	const struct HXbtree_node *node;
	unsigned int black_height = 0;
	char buf[80];

	printf("Test 6A: AMOV rebalancing\n");
	btree = generate_fixed_tree(7, 3, 9, 1, 0);
	/* Fixup colors for testcase (eww) */
	HXbtree_find(btree, "3")->color = NODE_RED;
	HXbtree_find(btree, "1")->color = NODE_BLACK;
	walk_tree(btree->root, buf, sizeof(buf));
	printf("\t" "Pre : %s\n", buf);

	/* Add a red node and trigger AMOV case 4 */
	HXbtree_add(btree, "5");
	walk_tree(btree->root, buf, sizeof(buf));
	printf("\t" "Post: %s\n", buf);

	printf("Test 6B: Black height\n");
	node = btree->root;
	while (node != NULL) {
		if (node->color == NODE_BLACK)
			++black_height;
		node = node->sub[0];
	}
	printf("\t" "Height to left-most node: %u\n", black_height);
	if (!test_6b(btree->root, black_height, 0))
		printf("\t" "...failed\n");

	HXbtree_free(btree);
	return;
}

int main(void)
{
	setvbuf(stdout, NULL, _IOLBF, 0);
	setvbuf(stderr, NULL, _IOLBF, 0);

	test_1(); /* allocates */
	test_2(); /* deallocates */
	test_3();
	test_5();
	test_6();
	//test_4();
	return EXIT_SUCCESS;
}

//-----------------------------------------------------------------------------
static struct HXbtree *generate_fixed_tree(unsigned int f, ...)
{
	struct HXbtree *b;
	char buf[Z_32];
	va_list argp;

	b = HXbtree_init(HXBT_CDATA | HXBT_CID | HXBT_CMPFN, strtolcmp);
	if (b == NULL)
		abort();

	va_start(argp, f);
	while (f != 0) {
		snprintf(buf, sizeof(buf), "%u", f);
		HXbtree_add(b, buf);
		f = va_arg(argp, unsigned int);
	}
	va_end(argp);
	return b;
}

static struct HXbtree *generate_perfect_tree(unsigned int height,
    unsigned int mult)
{
	unsigned int right = 1 << height;
	unsigned int incr  = right;
	unsigned int left  = incr / 2;
	unsigned int y, x;
	struct HXbtree *b;
	char buf[Z_32];

	b = HXbtree_init(HXBT_CDATA | HXBT_CID | HXBT_CMPFN, strtolcmp);
	if (b == NULL)
		abort();

	for (y = 0; y < height; ++y) {
		for(x = left; x < right; x += incr) {
			snprintf(buf, sizeof(buf), "%u", x * mult);
			HXbtree_add(b, buf);
		}
		incr /= 2;
		left /= 2;
	}

	return b;
}

static void height_check(const struct HXbtree *tree)
{
	double min, max, avg;
	min = log(tree->items + 1) / log(2);
	max = 2 * log(tree->items + 1) / log(2);
	avg = log((pow(2, min) + pow(2, max)) / 2) / log(2);
	printf("\t" "Item count: %u\n", tree->items);
	printf("\t" "Minimum height: %f\n", min);
	printf("\t" "Average height: %f\n", avg);
	printf("\t" "Maximum height: %f\n", max);
	printf("\t" "Current height: %u\n", tree_height(tree->root));
	return;
}

static int sbc_strcmp(const char *result, const char *expected)
{
	int ret = strcmp(result, expected);
	printf("\t" "Expected: %s\n", expected);
	printf("\t" "  Result: %s\n", result);
	if (ret != 0)
		printf("\t\t...failed\n");
	return ret;
}

static int strtolcmp(const void *a, const void *b)
{
	return strtol(a, NULL, 0) - strtol(b, NULL, 0);
}

static inline void timer_start(void)
{
	gettimeofday(&tv_start, NULL);
	printf("Timer started at %ld.%06ld\n",
	       tv_start.tv_sec, tv_start.tv_usec);
	return;
}

static inline void timer_end(void)
{
	struct timeval tv_end, delta;
	unsigned long sec;
	long acc;

	gettimeofday(&tv_end, NULL);
	printf("Timer ended at: %ld.%06ld\n",
	       tv_end.tv_sec, tv_start.tv_usec);

	sec = tv_end.tv_sec  - tv_start.tv_sec;
	acc = tv_end.tv_usec - tv_start.tv_usec;
	if(acc < 0) {
		delta.tv_sec  = sec - 1;
		delta.tv_usec = 1000000 + acc;
	} else {
		delta.tv_sec  = sec;
		delta.tv_usec = acc;
	}

	printf("Timer difference: %ld.%06ld\n", delta.tv_sec, delta.tv_usec);
	return;
}

static unsigned int tree_height(const struct HXbtree_node *node)
{
	unsigned int a = 1, b = 1;
	if(node->sub[0] != NULL)
		a += tree_height(node->sub[0]);
	if(node->sub[1] != NULL)
		b += tree_height(node->sub[1]);
	return (a > b) ? a : b;
}

static void __walk_tree(const struct HXbtree_node *node, char *buf, size_t s)
{
	int has_children = node->sub[0] != NULL || node->sub[1] != NULL;
	HX_strlcat(buf, node->key, s);

	if(node->color == 1)
		HX_strlcat(buf, "%b", s);
	if(has_children)
		HX_strlcat(buf, "(" /* ) */, s);
	if(node->sub[0] != NULL)
		__walk_tree(node->sub[0], buf, s);
	if(node->sub[1] != NULL) {
		HX_strlcat(buf, ",", s);
		__walk_tree(node->sub[1], buf, s);
	}
	if(has_children)
		HX_strlcat(buf, /* ( */ ")", s);
	return;
}

static void walk_tree(const struct HXbtree_node *node, char *buf, size_t s)
{
	*buf = '\0';
	__walk_tree(node, buf, s);
	return;
}

//=============================================================================
