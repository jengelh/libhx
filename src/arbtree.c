/*
 *	libHX/arbtree.c - Associative map with RB-tree
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2002 - 2008
 *
 *	This file is part of libHX. libHX is free software; you can
 *	redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <libHX/arbtree.h>
#include <libHX/string.h>
#include "internal.h"

enum {
	S_LEFT        = 0,
	S_RIGHT,
	NODE_RED      = 0,
	NODE_BLACK,
	HXBT_FLAGS_OK = HXBT_MAP | HXBT_CKEY | HXBT_CDATA | HXBT_CMPFN |
	                HXBT_ICMP | HXBT_SCMP | HXBT_CID,

	/* Allows for at least 16 million objects (in a worst-case tree) */
	BT_MAXDEP     = 48,
};

struct HXbtrav {
	const struct HXbtree *tree;
	struct HXbtree_node *current; /* last visited node */
	char *checkpoint;
	struct HXbtree_node *path[BT_MAXDEP]; /* stored path */
	unsigned char dir[BT_MAXDEP];
	unsigned int tid; /* last seen btree transaction */
	unsigned char depth;
};

static void btrav_checkpoint(struct HXbtrav *, const struct HXbtree_node *);
static struct HXbtree_node *btrav_next(struct HXbtrav *);
static struct HXbtree_node *btrav_rewalk(struct HXbtrav *);
static void btree_amov(struct HXbtree_node **, const unsigned char *,
	unsigned int, unsigned int *);
static unsigned int btree_del(struct HXbtree_node **, unsigned char *,
	unsigned int);
static void btree_dmov(struct HXbtree_node **, unsigned char *, unsigned int);
static void btree_free_dive(const struct HXbtree *, struct HXbtree_node *);
static int value_cmp(const void *, const void *);

//-----------------------------------------------------------------------------
EXPORT_SYMBOL struct HXbtree *HXbtree_init(unsigned int opts, ...)
{
	struct HXbtree *btree;
	va_list argp;
	va_start(argp, opts);

	BUILD_BUG_ON(offsetof(struct HXbtree, root) +
	             offsetof(struct HXbtree_node, sub[0]) !=
	             offsetof(struct HXbtree, root));

	if (opts & ~HXBT_FLAGS_OK)
		fprintf(stderr, "libHX-btree warning: unknown flags passed!\n");
	if ((btree = malloc(sizeof(struct HXbtree))) == NULL)
		return NULL;

	memset(btree, 0, sizeof(struct HXbtree));
	btree->opts  = opts;
	btree->items = 0;

	/*
	 * This should not be zero, otherwise the traverser functions will not
	 * start off correctly, since trav->tid is 0, but trav->tid must not
	 * equal btree->transact because that would mean the traverser is in
	 * sync with the tree.
	 */
	btree->tid = 1;

	if (opts & HXBT_CMPFN)
		btree->cmpfn = va_arg(argp, void *);
	else if (opts & HXBT_SCMP)
		btree->cmpfn = static_cast(void *, strcmp);
	else if (opts & HXBT_ICMP)
		btree->cmpfn = value_cmp;
	else if (opts & HXBT_MAP)
		btree->cmpfn = static_cast(void *, strcmp);
	else {
		fprintf(stderr,
		        "libHX-btree error: Your code does not use any of\n"
		        "- HXBT_CMPFN (you specify a comparison function)\n"
		        "- HXBT_SCMP (you specify us to take strcmp())\n"
		        "- HXBT_ICMP (you specify us to take value_cmp())\n"
		        "- HXBT_MAP (in which case, we would use strcmp() implicitly)\n"
		        "So we eventually ended up with no comparison function.\n"
		        "Please correct! Returning NULL for now.\n"
		);
		free(btree);
		va_end(argp);
		return NULL;
	}

	va_end(argp);
	return btree;
}

EXPORT_SYMBOL struct HXbtree_node *HXbtree_add(struct HXbtree *btree,
    const void *key, ...)
{
	struct HXbtree_node *node, *path[BT_MAXDEP];
	unsigned char dir[BT_MAXDEP];
	unsigned int depth = 0;
	const void *data;
	va_list argp;

	va_start(argp, key);
	data = va_arg(argp, const void *);
	va_end(argp);

	/*
	 * Since our struct HXbtree_node runs without a ->parent pointer,
	 * the path "upwards" from @node needs to be recorded somehow,
	 * here with @path. Another array, @dir is used to speedup direction
	 * decisions. (WP's "n->parent == grandparent(n)->left" is just slow.)
	 */
	path[depth]  = reinterpret_cast(struct HXbtree_node *, &btree->root);
	dir[depth++] = 0;
	node = btree->root;

	while (node != NULL) {
		int res = btree->cmpfn(key, node->key);
		if (res == 0) {
			if (!(btree->opts & HXBT_MAP)) {
				errno = EEXIST;
				return NULL;
			}

			/*
			 * The node already exists (found the key), overwrite
			 * the data.
			 */
			if (btree->opts & HXBT_CDATA) {
				void *p = &node->data;
				HX_strclone(p, data);
			} else {
				node->data = const_cast(void *, data);
			}

			return node;
		}

		res          = res > 0;
		path[depth]  = node;
		dir[depth++] = res;
		node         = node->sub[res];
	}

	if ((node = malloc(sizeof(struct HXbtree_node))) == NULL)
		return NULL;

	/*
	 * Add the node to the tree. In trying not to hit a rule 2 violation
	 * (each simple path has the same number of black nodes), it is colored
	 * red so that below we only need to check for rule 1 violations.
	 */
	node->sub[S_LEFT] = node->sub[S_RIGHT] = NULL;
	node->color = NODE_RED;
	path[depth-1]->sub[dir[depth-1]] = node;
	++btree->items;

	/* New node, push data into it */
	if (btree->opts & HXBT_MAP) {
		node->key  = (btree->opts & HXBT_CKEY) ?
		             HX_strdup(key) : const_cast(void *, key);
		node->data = (btree->opts & HXBT_CDATA) ?
		             HX_strdup(data) : const_cast(void *, data);
	} else {
		/* For convenience, node->key == node->data */
		if (btree->opts & HXBT_CDATA)
			node->key = node->data = HX_strdup(key);
		else
			node->key = node->data = const_cast(void *, key);
	}

	/*
	 * WP: [[Red-black_tree]] says:
	 * Case 1: @node is root node - just color it black (see below).
	 * Case 2: @parent is black - no action needed (skip).
	 * No rebalance needed for a 2-node tree.
	 */
	if (depth >= 3 && path[depth-1]->color == NODE_RED)
		btree_amov(path, dir, depth, &btree->tid);

	btree->root->color = NODE_BLACK;
	return node;
}

EXPORT_SYMBOL struct HXbtree_node *HXbtree_find(const struct HXbtree *btree,
    const void *key)
{
	struct HXbtree_node *node = btree->root;
	int res;

	while (node != NULL) {
		if ((res = btree->cmpfn(key, node->key)) == 0)
			return node;
		node = node->sub[res > 0];
	}

	return NULL;
}

EXPORT_SYMBOL void *HXbtree_get(const struct HXbtree *btree, const void *key)
{
	const struct HXbtree_node *node;
	if ((node = HXbtree_find(btree, key)) == NULL)
		return NULL;
	return node->data;
}

EXPORT_SYMBOL void *HXbtree_del(struct HXbtree *btree, const void *key)
{
	struct HXbtree_node *path[BT_MAXDEP], *node;
	unsigned char dir[BT_MAXDEP];
	unsigned int depth = 0;
	void *itemptr;

	if (btree->root == NULL)
		return NULL;

	path[depth]  = reinterpret_cast(struct HXbtree_node *, &btree->root);
	dir[depth++] = 0;
	node         = btree->root;

	while (node != NULL) {
		int res = btree->cmpfn(key, node->key);
		if (res == 0)
			break;
		res          = res > 0;
		path[depth]  = node;
		dir[depth++] = res;
		node         = node->sub[res];
	}

	if (node == NULL) {
		errno = ENOENT;
		return NULL;
	}
	itemptr = node->data;

	/* Removal of the node from the tree */
	--btree->items;
	++btree->tid;

	path[depth] = node;
	if (node->sub[S_RIGHT] == NULL)
		/* Simple case: No right subtree, replace by left subtree. */
		path[depth-1]->sub[dir[depth-1]] = node->sub[S_LEFT];
	else if (node->sub[S_LEFT] == NULL)
		/* Simple case: No left subtree, replace by right subtree. */
		path[depth-1]->sub[dir[depth-1]] = node->sub[S_RIGHT];
	else
		/*
		 * Find minimum/maximum element in right/left subtree and
		 * do appropriate deletion while updating @path and @depth.
		 */
		depth = btree_del(path, dir, depth);

	/*
	 * Deleting a red node does not violate either of the rules, so it is
	 * not necessary to rebalance in such a case.
	 */
	if (node->color == NODE_BLACK)
		btree_dmov(path, dir, depth);

	if (btree->opts & HXBT_MAP) {
		if (btree->opts & HXBT_CKEY)
			free(node->key);
		if (btree->opts & HXBT_CDATA)
			free(node->data);
	} else if (btree->opts & HXBT_CDATA) {
		/* remember, @node->key == @node->data, so only one free() */
		free(node->key);
	}

	free(node);
	errno = 0;
	/*
	 * In case %HXBT_CDATA was specified, the @itemptr value will be
	 * useless in most cases as it points to freed memory.
	 */
	return itemptr;
}

/*
 * Destroy the Btree and all its elements. This is just an introducer function
 * for btree_free_dive().
 */
EXPORT_SYMBOL void HXbtree_free(struct HXbtree *btree)
{
	if (btree->root != NULL)
		btree_free_dive(btree, btree->root);
	free(btree);
}

EXPORT_SYMBOL void *HXbtrav_init(const struct HXbtree *btree)
{
	struct HXbtrav *travp;
	if ((travp = calloc(1, sizeof(struct HXbtrav))) == NULL)
		return NULL;

	travp->tree = btree;
	return travp;
}

EXPORT_SYMBOL struct HXbtree_node *HXbtraverse(void *in)
{
	struct HXbtrav *travp = in;

	if (travp->tid != travp->tree->tid || travp->current == NULL)
		/*
		 * Every HXbtree operation that significantly changes the
		 * B-tree, increments @transact so we can decide here to
		 * rewalk.
		 */
		return btrav_rewalk(travp);
	else
		return btrav_next(travp);
}

EXPORT_SYMBOL void HXbtrav_free(void *in)
{
	struct HXbtrav *travp = in;

	if (travp->tree->opts & HXBT_CID)
		free(travp->checkpoint);
	free(travp);
}

//-----------------------------------------------------------------------------
static void btrav_checkpoint(struct HXbtrav *travp,
    const struct HXbtree_node *node)
{
	if (travp->tree->opts & HXBT_CID)
		HX_strclone(&travp->checkpoint, node->key);
	else
		travp->checkpoint = node->key;
}

static struct HXbtree_node *btrav_next(struct HXbtrav *trav)
{
	if (trav->current->sub[S_RIGHT] != NULL) {
		/* Got a right child */
		struct HXbtree_node *node;

		trav->dir[trav->depth++] = S_RIGHT;
		node = trav->current->sub[S_RIGHT];

		/* Which might have left childs (our inorder successors!) */
		while (node != NULL) {
			trav->path[trav->depth] = node;
			node = node->sub[S_LEFT];
			trav->dir[trav->depth++] = S_LEFT;
		}
		trav->current = trav->path[--trav->depth];
	} else if (trav->depth == 0) {
		/* No right child, no more parents */
		return trav->current = NULL;
	} else if (trav->dir[trav->depth-1] == S_LEFT) {
		/* We are the left child of the parent, move on to parent */
		trav->current = trav->path[--trav->depth];
	} else if (trav->dir[trav->depth-1] == S_RIGHT) {
		/*
		 * There is no right child, and we are the right child of the
		 * parent, so move on to the next inorder node (a distant
		 * parent). This works by walking up the path until we are the
		 * left child of a parent.
		 */
		while (true) {
			if (trav->depth == 0)
				/* No more parents */
				return trav->current = NULL;
			if (trav->dir[trav->depth-1] != S_RIGHT)
				break;
			--trav->depth;
		}
		trav->current = trav->path[--trav->depth];
	}

	btrav_checkpoint(trav, trav->current);
	return trav->current;
}

static struct HXbtree_node *btrav_rewalk(struct HXbtrav *trav)
{
	/*
	 * When the binary tree has been distorted (or the traverser is
	 * uninitilaized), by either addition or deletion of an object, our
	 * path recorded so far is (probably) invalid too. rewalk() will go and
	 * find the node we were last at.
	 */
	const struct HXbtree *btree = trav->tree;
	struct HXbtree_node *node   = btree->root;
	bool go_next = false;

	trav->depth = 0;

	if (trav->current == NULL) {
		/* Walk down the tree to the smallest element */
		while (node != NULL) {
			trav->path[trav->depth] = node;
			node = node->sub[S_LEFT];
			trav->dir[trav->depth++] = S_LEFT;
		}
	} else {
		/* Search for the specific node to rebegin traversal at. */
		const struct HXbtree_node *newpath[BT_MAXDEP];
		unsigned char newdir[BT_MAXDEP];
		int newdepth = 0, res;
		bool found = false;

		while (node != NULL) {
			newpath[newdepth] = trav->path[trav->depth] = node;
			res = btree->cmpfn(trav->checkpoint, node->key);
			if (res == 0) {
				++trav->depth;
				found = true;
				break;
			}
			res = res > 0;
			trav->dir[trav->depth++] = res;

			/*
			 * This (working) code gets 1st place in being totally
			 * cryptic without comments, so here goes:
			 *
			 * Right turns do not need to be saved, because we do
			 * not need to stop at that particular node again but
			 * can go directly to the next in-order successor,
			 * which must be a parent somewhere upwards where we
			 * did a left turn. If we only ever did right turns,
			 * we would be at the last node already.
			 *
			 * Imagine a 32-element perfect binary tree numbered
			 * from 1..32, and walk to 21 (directions: RLRL).
			 * The nodes stored are 24 and 22. btrav_next will
			 * go to 22, do 23, then jump _directly_ back to 24,
			 * omitting the redundant check at 20.
			 */
			if (res == S_LEFT)
				newdir[newdepth++] = S_LEFT;

			node = node->sub[res];
		}

		if (found) {
			/*
			 * We found the node, but which HXbtraverse() has
			 * already returned. Advance to the next inorder node.
			 * (Code needs to come after @current assignment.)
			 */
			go_next = true;
		} else {
			/*
			 * If the node travp->current is actually deleted (@res
			 * will never be 0 above), traversal re-begins at the
			 * next inorder node, which happens to be the last node
			 * we turned left at.
			 */
			memcpy(trav->path, newpath, sizeof(trav->path));
			memcpy(trav->dir, newdir, sizeof(trav->dir));
			trav->depth = newdepth;
		}
	}

	if (trav->depth == 0) {
		/* no more elements */
		trav->current = NULL;
	} else {
		trav->current = trav->path[--trav->depth];
		if (trav->current == NULL)
			fprintf(stderr, "btrav_rewalk: problem: current==NULL\n");
		btrav_checkpoint(trav, trav->current);
	}

	trav->tid = btree->tid;
	if (go_next)
		return btrav_next(trav);
	else
		return trav->current;
}

static void btree_amov(struct HXbtree_node **path, const unsigned char *dir,
    unsigned int depth, unsigned int *tid)
{
	struct HXbtree_node *uncle, *parent, *grandp, *newnode;

	/*
	 * The newly inserted node (or the last rebalanced node) at
	 * @path[depth-1] is red, so the parent must not be.
	 *
	 * Use an iterative approach to not waste time with recursive function
	 * calls. The @LR variable is used to handle the symmetric case without
	 * code duplication.
	 */
	do {
		unsigned int LR = dir[depth-2];

		grandp = path[depth-2];
		parent = path[depth-1];
		uncle  = grandp->sub[!LR];

		if (uncle != NULL && uncle->color == NODE_RED) {
			/*
			 * Case 3 (WP): Only colors have to be swapped to keep
			 * the black height. But rebalance needs to continue.
			 */
			parent->color = NODE_BLACK;
			uncle->color  = NODE_BLACK;
			grandp->color = NODE_RED;
			depth        -= 2;
			continue;
		}

		/*
		 * Case 4 (WP): New node is the right child of its parent, and
		 * the parent is the left child of the grandparent. A left
		 * rotate is done at the parent to transform it into a case 5.
		 */
		if (dir[depth-1] != LR) {
			newnode          = parent->sub[!LR];
			parent->sub[!LR] = newnode->sub[LR];
			newnode->sub[LR] = parent;
			grandp->sub[LR]  = newnode;
			/* relabel */
			parent  = grandp->sub[LR];
			newnode = parent->sub[LR];
		} else {
			newnode = path[depth];
		}

		/*
		 * Case 5: New node is the @LR child of its parent which is
		 * the @LR child of the grandparent. A right rotation on
		 * @grandp is performed.
		 */
		grandp->sub[LR]  = parent->sub[!LR];
		parent->sub[!LR] = grandp;
		path[depth-3]->sub[dir[depth-3]] = parent;
		grandp->color    = NODE_RED;
		parent->color    = NODE_BLACK;
		++*tid;
		break;
	} while (depth >= 3 && path[depth-1]->color == NODE_RED);
}

static unsigned int btree_del(struct HXbtree_node **path, unsigned char *dir,
    unsigned int depth)
{
	/* Both subtrees exist */
	struct HXbtree_node *io_node, *io_parent, *orig_node = path[depth];
	unsigned char color;
	unsigned int spos;

	io_node    = orig_node->sub[S_RIGHT];
	dir[depth] = S_RIGHT;

	if (io_node->sub[S_LEFT] == NULL) {
		/* Right subtree node is direct inorder */
		io_node->sub[S_LEFT] = orig_node->sub[S_LEFT];
		color                = io_node->color;
		io_node->color       = orig_node->color;
		orig_node->color     = color;

		path[depth-1]->sub[dir[depth-1]] = io_node;
		path[depth++]        = io_node;
		return depth;
	}

	/*
	 * Walk down to the leftmost element, keep track of inorder node
	 * and its parent.
	 */
	spos = depth++;

	do {
		io_parent    = io_node;
		path[depth]  = io_parent;
		dir[depth++] = S_LEFT;
		io_node      = io_parent->sub[S_LEFT];
	} while (io_node->sub[S_LEFT] != NULL);

	/* move node up */
	path[spos-1]->sub[dir[spos-1]] = path[spos] = io_node;
	io_parent->sub[S_LEFT]         = io_node->sub[S_RIGHT];
	io_node->sub[S_LEFT]           = orig_node->sub[S_LEFT];
	io_node->sub[S_RIGHT]          = orig_node->sub[S_RIGHT];

	color          = io_node->color;
	io_node->color = orig_node->color;

	/*
	 * The nodes (@io_node and @orig_node) have been swapped. While
	 * @orig_node has no pointers to it, it still exists and decisions are
	 * made upon its properties in HXbtree_del() and btree_dmov() until it
	 * is freed later. Hence we need to keep the color.
	 */
	orig_node->color = color;
	return depth;
}

static void btree_dmov(struct HXbtree_node **path, unsigned char *dir,
    unsigned int depth)
{
	struct HXbtree_node *w, *x;

	while (true) {
		unsigned char LR = dir[depth - 1];
		x = path[depth - 1]->sub[LR];

		if (x != NULL && x->color == NODE_RED) {
			/* (WP) "delete_one_child" */
			x->color = NODE_BLACK;
			break;
		}

		if (depth < 2)
			/* Case 1 */
			break;

		/* @w is the sibling of @x (the current node). */
		w = path[depth - 1]->sub[!LR];
		if (w->color == NODE_RED) {
			/*
			 * Case 2. @w is of color red. In order to collapse
			 * cases, a left rotate is performed at @x's parent and
			 * colors are swapped to make @w a black node.
			 */
			w->color = NODE_BLACK;
			path[depth - 1]->color = NODE_RED;
			path[depth - 1]->sub[!LR] = w->sub[LR];
			w->sub[LR] = path[depth - 1];
			path[depth - 2]->sub[dir[depth - 2]] = w;
			path[depth] = path[depth - 1];
			dir[depth]  = LR;
			path[depth - 1] = w;
			w = path[++depth - 1]->sub[!LR];
		}

		if ((w->sub[LR] == NULL || w->sub[LR]->color == NODE_BLACK) &&
		   (w->sub[!LR] == NULL || w->sub[!LR]->color == NODE_BLACK)) {
			/* Case 3/4: @w has no red children. */
			w->color = NODE_RED;
			--depth;
			continue;
		}

		if (w->sub[!LR] == NULL || w->sub[!LR]->color == NODE_BLACK) {
			/* Case 5 */
			struct HXbtree_node *y = w->sub[LR];
			y->color = NODE_BLACK;
			w->color = NODE_RED;
			w->sub[LR] = y->sub[!LR];
			y->sub[!LR] = w;
			w = path[depth - 1]->sub[!LR] = y;
		}

		/* Case 6 */
		w->color = path[depth - 1]->color;
		path[depth - 1]->color = NODE_BLACK;
		w->sub[!LR]->color = NODE_BLACK;
		path[depth - 1]->sub[!LR] = w->sub[LR];
		w->sub[LR] = path[depth - 1];
		path[depth - 2]->sub[dir[depth - 2]] = w;
		break;
	}
}

static void btree_free_dive(const struct HXbtree *btree,
    struct HXbtree_node *node)
{
	/*
	 * Recursively dives into the tree and destroys elements. Note that you
	 * shall use this when destroying a complete tree instead of iterated
	 * deletion with HXbtree_del(). Since this functions is meant to free
	 * it all, it does not need to care about rebalancing.
	 */
	if (node->sub[S_LEFT] != NULL)
		btree_free_dive(btree, node->sub[S_LEFT]);
	if (node->sub[S_RIGHT] != NULL)
		btree_free_dive(btree, node->sub[S_RIGHT]);

	if (btree->opts & HXBT_MAP) {
		if (btree->opts & HXBT_CKEY)
			free(node->key);
		if (btree->opts & HXBT_CDATA)
			free(node->data);
	} else if (btree->opts & HXBT_CDATA) {
		/* remember, node->key == node->data, so only one free() */
		free(node->key);
	}

	free(node);
}

static int value_cmp(const void *pa, const void *pb)
{
	return pa - pb;
}
