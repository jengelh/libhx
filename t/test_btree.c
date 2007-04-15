#include <sys/time.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX.h>

static int tree_height(struct HXbtree_node *);
static void walk_tree(struct HXbtree_node *, char *, size_t);
static long walk_tree_i(struct HXbtree_node *, char *, size_t);

static const char *const Color[] = {"RED", "BLACK"};
static struct HXbtree *btree;
static struct timeval tv_start;

//-----------------------------------------------------------------------------
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

static void test_1(void)
{
	/*
	        d
	      /   \
	    b       g
	   / \     / \
	  a   c   f   h
	*/
	static const char *const targets[] =
		{"d", "b", "g", "a", "c", "f", "h", NULL};
	const char *const *t = targets;
	printf("Test #1: Creating ordered tree with 7 elements (hg 3)\n");
	btree = HXbtree_init(HXBT_MAP);

	while(*t != NULL) {
		HXbtree_add(btree, *t);
		++t;
	}
	return;
}

static void test_2(void)
{
	char buf[80];
	buf[0] = '\0';
	printf("Test #2: Manual traversion, check for correct positions & colors\n");

	walk_tree(btree->root, buf, sizeof(buf));
	if(strcmp(buf, "d%b(b%b(a,c),g%b(f,h))") != 0)
		printf("...failed\n");
	return;
}

static void test_3(void)
{
	char buf[80];
	buf[0] = '\0';
	printf("Test #3: Node insertion and test for positions/colors\n");

	HXbtree_add(btree, "e", NULL);
	walk_tree(btree->root, buf, sizeof(buf));
	if(strcmp(buf, "d%b(b%b(a,c),g(f%b(e),h%b))") != 0)
	    printf("...failed\n");

	/*      d
	      /   \
	    b       g
	   / \     / \
	  a   c   f   h
	         /
	        e
	*/
	return;
}

static void test_4(void)
{
	double min, max;
	min = log(btree->items + 1) / log(2);
	max = 2 * log(btree->items + 1) / log(2);
	printf("Test #4: Height check (item count: %lu)\n", btree->items);
	printf("\t" "Minimum height: %f\n", min);
	printf("\t" "Average height: %f\n",
	       log((pow(2, min) + pow(2, max)) / 2) / log(2));
	printf("\t" "Maximum height: %f\n", max);
	printf("\t" "Current height: %u\n", tree_height(btree->root));
	return;
}

static void test_5(void)
{
	struct HXbtrav *travp;
	struct HXbtree_node *node;

	printf("Test #5: Standard traverse\n");
	travp = HXbtrav_init(btree);
	while((node = HXbtraverse(travp)) != NULL)
	    printf("\t" "key: %s (%s)\n", (const char *)node->key,
	           Color[node->color]);
	HXbtrav_free(travp);
	return;
}

static void test_6(void)
{
	char buf[80];
	buf[0] = '\0';
	printf("Test #6: B-tree node deletion\n");

	HXbtree_del(btree, "d");
	walk_tree(btree->root, buf, sizeof(buf));
	if(strcmp(buf, "e%b(b%b(a,c),g(f%b,h%b))") != 0)
		printf("...failed\n");

	/*      e       (D replaced by its in-order successor E)
	      /   \
	    b       g
	   / \     / \
	  a   c   f   h
	*/
	return;
}

static void test_7(void)
{
	struct HXbtrav *travp = HXbtrav_init(btree);
	const struct HXbtree_node *node;

	printf("Test #7: Traverse with B-tree change\n");
	while((node = HXbtraverse(travp)) != NULL) {
		char buf[80];
		buf[0] = '\0';
		walk_tree(btree->root, buf, sizeof(buf));
		printf("\t" "stamp: %s\n", buf);
		printf("\t" "  key: %s (%s)\n",
		       (const char *)node->key, Color[node->color]);
		if(*(const char *)node->key == 'b') {
			printf("\t" "Deleting [current] node B\n");
			HXbtree_del(btree, "b");
		}
		if(*(const char *)node->key == 'f') {
			printf("\t" "Deleting [next] node G\n");
			HXbtree_del(btree, "g");
		}
	}

	HXbtrav_free(travp);
	/*      e
	      /   \
	    c       h
	   /       /
	  a       f
	*/
	return;
}

static void test_8(void)
{
	struct HXbtrav *travp = HXbtrav_init(btree);
	const struct HXbtree_node *node;

	printf("Test #8: Traverse with B-tree destruction\n");
	while((node = HXbtraverse(travp)) != NULL) {
		printf("\t" "About to delete \"%s\"\n",
		       (const char *)node->key);
		HXbtree_del(btree, node->key);
	}

	HXbtrav_free(travp);
	printf("\t" "Elements: %lu (should be 0)\n", btree->items);
	printf("\t" "Root: %p (should be NIL/NULL/0)\n", btree->root);
	return;
}

static void test_9(void)
{
	struct HXbtrav *travp;
	printf("Test #9: Traversing empty tree\n");
	travp = HXbtrav_init(btree);
	while(HXbtraverse(travp) != NULL);
	HXbtrav_free(travp);
	return;
}

static void test_10(void)
{
	int i;
	printf("Test #10: Worst case fill-up with tree height check\n");
	btree = HXbtree_init(0);

	for(i = 1; i <= 65536; ++i) {
		int hg;

		HXbtree_add(btree, (void *)i);
		hg = tree_height(btree->root);

		printf("\t" "IN=%6d  MIN=%2f  HG=%2d  MAX=%2f\n",
		       i, log(btree->items + 1) / log(2), hg,
		       2 * log(btree->items + 1));
	}

	printf("\t" "Objects in the tree: %lu\n", btree->items);
	if(btree->items != i - 1)
		printf("...failed\n");
	return;
}

static void test_11(void)
{
	/*
	                8
	          .----' `----.
	        4               12
	      /   \          /     \
	    2       6      10       14      . . .
	   / \     / \    /  \     / \
	  1   3   5   7  9    11  13  15
	*/
	static const long targets[] = {16, 8, 24, 4, 12, 20, 28, 2, 6, 10, 14,
		18, 22, 26, 30, 1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25,
		27, 29, 31, 0};
	const long *t = targets;
	printf("Test #11: Creating ordered tree with 15 elements (hg 4)\n"
	       "\tand testing right-end walking\n");
	btree = HXbtree_init(HXBT_ICMP);

	while(*t != 0) {
		HXbtree_add(btree, (const void *)*t);
		++t;
	}

	printf("\tWalking until 9\n");
	void *trav = HXbtrav_init(btree);
	struct HXbtree_node *nd;
	while((nd = HXbtraverse(trav)) != NULL) {
		if((long)nd->key == 9)
			break;
	}
	printf("\tDeleting 9\n");
	HXbtree_del(btree, (const void *)9);
	printf("\tContinuing traversal:");
	while((nd = HXbtraverse(trav)) != NULL)
		printf(" %lu", (long)nd->key);
	HXbtrav_free(trav);
	printf("\n");
	return;
}

static void test_12(void)
{
	unsigned long n;
	int hg;

	printf("Test #12: Tree height expansion check\n");
	btree = HXbtree_init(HXBT_ICMP);

	timer_start();
	for(n = 1; n != 0; ++n) {
		HXbtree_add(btree, (const void *)n);
		if((n & 0xFFFFF) != 0)
			continue;
		hg = tree_height(btree->root);
		printf("\t%lu objects, height %d\n", btree->items, hg);
		if(hg == 47)
			break;
	}
	timer_end();

	return;
}

static void test_13(void)
{
	unsigned long n;
	int i;

	printf("Test #13: Lookup speed\n");
	for(i = 0; i < 5; ++i) {
		timer_start();
		for(n = btree->items; n >= 1; --n)
			HXbtree_find(btree, (const void *)n);
		timer_end();
	}

	return;
}

int main(void)
{
	setvbuf(stdout, NULL, _IOLBF, 0);
	setvbuf(stderr, NULL, _IOLBF, 0);
	test_1();
	test_2();
	test_3();
	test_4();
	test_5();
	test_6();
	test_7();
	test_8();
	test_9();
	//test_10();
	HXbtree_free(btree);
	printf("Freeing tree: With a memory debugger, check the leaks now.\n");
	test_11();
	HXbtree_free(btree);
	test_12();
	test_13();
	HXbtree_free(btree);
	return EXIT_SUCCESS;
}

//-----------------------------------------------------------------------------
static int tree_height(struct HXbtree_node *node)
{
	int a = 1, b = 1;
	if(node->s[0] != NULL)
		a += tree_height(node->s[0]);
	if(node->s[1] != NULL)
		b += tree_height(node->s[1]);
	return (a > b) ? a : b;
}

static void walk_tree(struct HXbtree_node *node, char *buf, size_t s)
{
	int hc = node->s[0] != NULL || node->s[1] != NULL;
	HX_strlcat(buf, node->key, s);

	if(node->color == 1)
		HX_strlcat(buf, "%b", s);
	if(hc)
		HX_strlcat(buf, "(" /* ) */, s);
	if(node->s[0] != NULL)
		walk_tree(node->s[0], buf, s);
	if(node->s[1] != NULL) {
		HX_strlcat(buf, ",", s);
		walk_tree(node->s[1], buf, s);
	}
	if(hc)
		HX_strlcat(buf, /* ( */ ")", s);
	return;
}

static long walk_tree_i(struct HXbtree_node *node, char *buf, size_t s)
{
	int hc = node->s[0] != NULL || node->s[1] != NULL;
	char num[32];
	int spc = 0;

	snprintf(num, 32, "%lx", (unsigned long)node->key);
	HX_strlcat(buf, num, s);
	spc += strlen(num);

	if(node->color == 1)
		HX_strlcat(buf, "%b", s); spc += 2;
	if(hc) {
		HX_strlcat(buf, "(" /* ) */, s);
		++spc;
	}
	if(node->s[0] != NULL)
		spc += walk_tree_i(node->s[0], buf, s);
	if(node->s[1] != NULL) {
		HX_strlcat(buf, ",", s);
		spc += 1 + walk_tree_i(node->s[1], buf, s);
	}
	if(hc) {
		HX_strlcat(buf, /* ( */ ")", s);
		++spc;
	}
	return spc;
}

//=============================================================================
