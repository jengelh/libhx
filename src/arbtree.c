/*
	libHX/arbtree.c - Associative RB-tree
	Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2002 - 2007

	This file is part of libHX. libHX is free software; you can
	redistribute it and/or modify it under the terms of the GNU
	Lesser General Public License as published by the Free Software
	Foundation; however ONLY version 2 of the License. For details,
	see the file named "LICENSE.LGPL2".
*/
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "libHX.h"

enum {
	S_LEFT        = 0,
	S_RIGHT,
	NODE_RED      = 0,
	NODE_BLACK,
	HXBT_FLAGS_OK = HXBT_MAP | HXBT_CKEY | HXBT_CDATA | HXBT_CMPFN |
	                HXBT_ICMP | HXBT_SCMP | HXBT_CID,

	/* Allow for a worst-case tree with max. 16 million objects */
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

	if(opts & ~HXBT_FLAGS_OK)
		fprintf(stderr, "libHX-btree warning: unknown flags passed!\n");
	if((btree = malloc(sizeof(struct HXbtree))) == NULL)
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

	if(opts & HXBT_CMPFN)     btree->cmpfn = va_arg(argp, void *);
	else if(opts & HXBT_SCMP) btree->cmpfn = static_cast(void *, strcmp);
	else if(opts & HXBT_ICMP) btree->cmpfn = value_cmp;
	else if(opts & HXBT_MAP)  btree->cmpfn = static_cast(void *, strcmp);
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

	path[depth]  = reinterpret_cast(struct HXbtree_node *, &btree->root);
	dir[depth++] = 0;
	node = btree->root;

	while(node != NULL) {
		int res = btree->cmpfn(key, node->key);
		if(res == 0) {
			if(!(btree->opts & HXBT_MAP)) {
				errno = EEXIST;
				return NULL;
			}

			/*
			 * The node already exists (found the key), overwrite
			 * the data.
			 */
			if(btree->opts & HXBT_CDATA) {
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

	if((node = malloc(sizeof(struct HXbtree_node))) == NULL)
		return NULL;

	/*
	 * Add the node to the tree. In trying not to hit a rule 2 violation
	 * (each simple path has the same number of black nodes), it is colored
	 * red so that below we only need to check for rule 1 violations.
	 */
	node->sub[S_LEFT] = node->sub[S_RIGHT] = NULL;
	node->color = NODE_RED;
	path[depth - 1]->sub[dir[depth - 1]] = node;
	++btree->items;

	/* New node, push data into it */
	if(btree->opts & HXBT_MAP) {
		node->key  = (btree->opts & HXBT_CKEY) ?
		             HX_strdup(key) : const_cast(void *, key);
		node->data = (btree->opts & HXBT_CDATA) ?
		             HX_strdup(data) : const_cast(void *, data);
	} else {
		/* For convenience, node->key == node->data */
		if(btree->opts & HXBT_CDATA)
			node->key = node->data = HX_strdup(key);
		else
			node->key = node->data = const_cast(void *, key);
	}

	if(depth >= 3 && path[depth - 1]->color == NODE_RED)
		btree_amov(path, dir, depth, &btree->tid);

	btree->root->color = NODE_BLACK;
	return node;
}

EXPORT_SYMBOL struct HXbtree_node *HXbtree_find(const struct HXbtree *btree,
    const void *key)
{
	struct HXbtree_node *node = btree->root;
	int res;

	while(node != NULL) {
		if((res = btree->cmpfn(key, node->key)) == 0)
			return node;
		node = node->sub[res > 0];
	}

	return NULL;
}

EXPORT_SYMBOL void *HXbtree_get(const struct HXbtree *btree, const void *key)
{
	const struct HXbtree_node *node;
	if((node = HXbtree_find(btree, key)) == NULL)
		return NULL;
	return node->data;
}

EXPORT_SYMBOL void *HXbtree_del(struct HXbtree *btree, const void *key)
{
	struct HXbtree_node *path[BT_MAXDEP], *node;
	unsigned char dir[BT_MAXDEP];
	unsigned int depth = 0;
	void *itemptr;

	if(btree->root == NULL)
		return NULL;

	path[depth]  = reinterpret_cast(struct HXbtree_node *, &btree->root);
	dir[depth++] = 0;
	node         = btree->root;

	while(node != NULL) {
		int res = btree->cmpfn(key, node->key);
		if(res == 0)
			break;
		res          = res > 0;
		path[depth]  = node;
		dir[depth++] = res;
		node         = node->sub[res];
	}

	if(node == NULL) {
		errno = ENOENT;
		return NULL;
	}
	itemptr = node->data;

	/* Removal of the node from the tree */
	--btree->items;
	++btree->tid;

	path[depth] = node;
	if(node->sub[S_RIGHT] == NULL)
		/* Simple case: No right subtree, replace by left subtree. */
		path[depth-1]->sub[dir[depth-1]] = node->sub[S_LEFT];
	else if(node->sub[S_LEFT] == NULL)
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
	if(node->color == NODE_BLACK)
		btree_dmov(path, dir, depth);

	if(btree->opts & HXBT_MAP) {
		if(btree->opts & HXBT_CKEY)
			free(node->key);
		if(btree->opts & HXBT_CDATA)
			free(node->data);
	} else if(btree->opts & HXBT_CDATA) {
		/* remember, node->key == node->data, so only one free() */
		free(node->key);
	}

	free(node);
	errno = 0;
	return itemptr;
}

/*
 * Destroy the Btree and all its elements. This is just an introducer function
 * for btree_free_dive().
 */
EXPORT_SYMBOL void HXbtree_free(struct HXbtree *btree)
{
	if(btree->root != NULL)
		btree_free_dive(btree, btree->root);
	free(btree);
	return;
}

EXPORT_SYMBOL void *HXbtrav_init(const struct HXbtree *btree)
{
	struct HXbtrav *travp;
	if((travp = calloc(1, sizeof(struct HXbtrav))) == NULL)
		return NULL;

	travp->tree = btree;
	return travp;
}

EXPORT_SYMBOL struct HXbtree_node *HXbtraverse(void *in)
{
	struct HXbtrav *travp = in;
	if(travp->tid != travp->tree->tid || travp->current == NULL)
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
	if(travp->tree->opts & HXBT_CID)
		free(travp->checkpoint);
	free(travp);
	return;
}

//-----------------------------------------------------------------------------
static void btrav_checkpoint(struct HXbtrav *travp,
    const struct HXbtree_node *node)
{
	if(travp->tree->opts & HXBT_CID)
		HX_strclone(&travp->checkpoint, node->key);
	else
		travp->checkpoint = node->key;
	return;
}

static struct HXbtree_node *btrav_next(struct HXbtrav *trav)
{
	if(trav->current->sub[S_RIGHT] != NULL) {
		/* Got a right child */
		struct HXbtree_node *node;
		trav->dir[trav->depth++] = S_RIGHT;
		node = trav->current->sub[S_RIGHT];

		/* Which might have left childs (our inorder successors!) */
		while(node != NULL) {
			trav->path[trav->depth] = node;
			node = node->sub[S_LEFT];
			trav->dir[trav->depth++] = S_LEFT;
		}
		trav->current = trav->path[--trav->depth];
	} else if(trav->depth == 0) {
		/* No right child, no more parents */
		return trav->current = NULL;
	} else if(trav->dir[trav->depth-1] == S_LEFT) {
		/* We are the left child of the parent, move on to parent */
		trav->current = trav->path[--trav->depth];
	} else if(trav->dir[trav->depth-1] == S_RIGHT) {
		/*
		 * There is no right child, and we are the right child of the
		 * parent, so move on to the next inorder node (a distant
		 * parent). This works by walking up the path until we are the
		 * left child of a parent.
		 */
		while(1) {
			if(trav->depth == 0)
				/* No more parents */
				return trav->current = NULL;
			if(trav->dir[trav->depth-1] != S_RIGHT)
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
	unsigned int go_next = 0;

	trav->depth = 0;

	if(trav->current == NULL) {
		/* Walk down the tree to the smallest element */
		while(node != NULL) {
			trav->path[trav->depth] = node;
			node = node->sub[S_LEFT];
			trav->dir[trav->depth++] = S_LEFT;
		}
	} else {
		/* Search for the specific node to rebegin traversal at. */
		const struct HXbtree_node *newpath[BT_MAXDEP];
		unsigned char newdir[BT_MAXDEP];
		int found = 0, newdepth = 0, res;

		while(node != NULL) {
			newpath[newdepth] = trav->path[trav->depth] = node;
			res = btree->cmpfn(trav->checkpoint, node->key);
			if(res == 0) {
				++trav->depth;
				++found;
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

		if(found) {
			/*
			 * We found the node, but which HXbtraverse() has
			 * already returned. Advance to the next inorder node.
			 * (Code needs to come after @current assignment.)
			 */
			go_next = 1;
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

	if(trav->depth == 0) {
		/* no more elements */
		trav->current = NULL;
	} else {
		trav->current = trav->path[--trav->depth];
		if(trav->current == NULL)
			fprintf(stderr, "btrav_rewalk: problem: current==NULL\n");
		btrav_checkpoint(trav, trav->current);
	}

	trav->tid = btree->tid;
	if(go_next)
		return btrav_next(trav);
	else
		return trav->current;
}

static void btree_amov(struct HXbtree_node **path, const unsigned char *dir,
    unsigned int depth, unsigned int *tid)
{
	struct HXbtree_node *x, *y;

	/*
	 * The newly inserted node (or the last rebalanced node) (called Q,
	 * even if not referenced in the code) is red, so the parent may not
	 * be.
	 */
	do {
		unsigned char LR = dir[depth - 2];
		y = path[depth - 2]->sub[!LR];

		if(y != NULL && y->color == NODE_RED) {
			/*
			 * Case 1: @q's uncle @y (@Q->parent->sibling or
			 * @q->parent->parent->child[N] is red. In this case,
			 * only colors have to be swapped to keep the black
			 * height.
			 */
			path[depth - 1]->color = y->color = NODE_BLACK;
			path[depth - 2]->color = NODE_RED;
			depth -= 2;
			continue;
		}

		if(dir[depth - 1] == LR) {
			/* Preparation for a pure case 2: @y is @q's parent */
			y = path[depth - 1];
		} else {
			/*
			 * Case 3: @q is the !LR child of its parent. A right
			 * rotate is done at @x to transform it into a case 2.
			 */
			x = path[depth - 1];
			y = x->sub[!LR];
			x->sub[!LR] = y->sub[LR];
			y->sub[LR]  = x;
			path[depth - 2]->sub[LR] = y;
		}

		/*
		 * Case 2: @q is the LR child of its parent. @x is @q's
		 * grandparent. A right rotation at @y is performed, since it
		 * does not change the black height nor the subtree's (seen
		 * from @x) root color. (@x is the root before, @y after the
		 * rotate.) */
		x = path[depth - 2];
		x->sub[LR]  = y->sub[!LR];
		y->sub[!LR] = x;
		path[depth - 3]->sub[dir[depth - 3]] = y;
		x->color   = NODE_RED;
		y->color   = NODE_BLACK;
		++*tid;
		break;
	} while(depth >= 3 && path[depth - 1]->color == NODE_RED);

	return;
}

static unsigned int btree_del(struct HXbtree_node **path, unsigned char *dir,
    unsigned int depth)
{
	struct HXbtree_node *s, *p = path[depth], *r = p->sub[S_RIGHT];
	unsigned char cl;

	if(r->sub[S_LEFT] != NULL) {
		/*
		 * Case 2A: @p's right child @r has a left child. Search for
		 * the inordner node @s and move it to @p's position.
		 */
		unsigned int spos = depth++;

		while(1) {
			path[depth]  = r;
			dir[depth++] = S_LEFT;
			if((s = r->sub[S_LEFT])->sub[S_LEFT] == NULL)
				break;
			r = s;
		}

		path[spos] = s;
		dir[spos]  = S_RIGHT;
		path[spos - 1]->sub[dir[spos - 1]] = s;
		s->sub[S_LEFT]  = p->sub[S_LEFT];
		r->sub[S_LEFT]  = s->sub[S_RIGHT];
		s->sub[S_RIGHT] = p->sub[S_RIGHT];

		/*
		 * The only purpose of keeping the old color (afterwards in
		 * P->color) is so the "if(node->color == NODE_BLACK)"
		 * expression back in HXbtree_del() can work.
		 */
		cl = s->color;
		s->color = p->color;
		p->color = cl;
		return depth;
	}

	/*
	 * Case 2: @p's right child @r has no left child. In this case, @r's
	 * right subtree is pulled up and placed where @p was. @r will get @p's
	 * left subtree.
	 */
	r->sub[S_LEFT] = p->sub[S_LEFT];
	cl = r->color;
	r->color = p->color;
	p->color = cl;
	path[depth - 1]->sub[dir[depth - 1]] = r;
	path[depth]  = r;
	dir[depth++] = S_RIGHT;
	return depth;
}

static void btree_dmov(struct HXbtree_node **path, unsigned char *dir,
    unsigned int depth)
{
	struct HXbtree_node *w, *x;

	while(1) {
		unsigned char LR = dir[depth - 1];
		x = path[depth - 1]->sub[LR];

		if(x != NULL && x->color == NODE_RED) {
			/* Case 1: */
			x->color = NODE_BLACK;
			break;
		}

		if(depth < 2)
			break;

		/* @w is the sibling of @x (the current node). */
		w = path[depth - 1]->sub[!LR];
		if(w->color == NODE_RED) {
			/*
			 * Extra case: @w is of color red. In order to collapse
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
			/* 2A, 3AA, 3B */
		}

		if((w->sub[LR] == NULL || w->sub[LR]->color == NODE_BLACK) &&
		  (w->sub[!LR] == NULL || w->sub[!LR]->color == NODE_BLACK)) {
			/* Case 2: @w has no red children. */
			w->color = NODE_RED;
			--depth;
			continue;
		}

		if(w->sub[!LR] == NULL || w->sub[!LR]->color == NODE_BLACK) {
			/* Case 3A: @w's LR child is red */
			struct HXbtree_node *y = w->sub[LR];
			y->color = NODE_BLACK;
			w->color = NODE_RED;
			w->sub[LR] = y->sub[!LR];
			y->sub[!LR] = w;
			w = path[depth - 1]->sub[!LR] = y;
		}

		/* Case 3: @w's !LR child is red */
		w->color = path[depth - 1]->color;
		path[depth - 1]->color = NODE_BLACK;
		w->sub[!LR]->color = NODE_BLACK;
		path[depth - 1]->sub[!LR] = w->sub[LR];
		w->sub[LR] = path[depth - 1];
		path[depth - 2]->sub[dir[depth - 2]] = w;
		break;
	}
	return;
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
	if(node->sub[S_LEFT] != NULL)
		btree_free_dive(btree, node->sub[S_LEFT]);
	if(node->sub[S_RIGHT] != NULL)
		btree_free_dive(btree, node->sub[S_RIGHT]);

	if(btree->opts & HXBT_MAP) {
		if(btree->opts & HXBT_CKEY)
			free(node->key);
		if(btree->opts & HXBT_CDATA)
			free(node->data);
	} else if(btree->opts & HXBT_CDATA) {
		/* remember, node->key == node->data, so only one free() */
		free(node->key);
	}

	free(node);
	return;
}

static int value_cmp(const void *pa, const void *pb)
{
	return pa - pb;
}

//=============================================================================
