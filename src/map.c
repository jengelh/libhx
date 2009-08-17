/*
 *	Maps (key-value pairs)
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2009
 *
 *	This file is part of libHX. libHX is free software; you can
 *	redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 *
 *	Incorporates Public Domain code from Bob Jenkins's lookup3 (May 2006)
 */
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libHX/list.h>
#include <libHX/map.h>
#include <libHX/string.h>
#include "internal.h"

/**
 * %HX_MAPTYPE_HASH:	struct HXhmap
 */
enum HXmap_type {
	HX_MAPTYPE_HASH = 1,
};

/**
 * @type:	actual type of map (%HX_MAPTYPE_*), used for virtual calls
 * @ops:	function pointers for key and data management
 */
struct HXmap_private {
	/* from struct HXmap */
	unsigned int items, flags;

	/* private: */
	enum HXmap_type type;
	size_t key_size, data_size;
	struct HXmap_ops ops;
};

/**
 * @bk_array:	bucket pointers
 * @power:	index into HXhash_primes to denote number of buckets
 * @max_load:	maximum number of elements before table gets enlarged
 * @min_load:	minimum number of elements before table gets shrunk
 * @tid:	transaction ID, used to track relayouts
 */
struct HXhmap {
	struct HXmap_private super;

	struct HXlist_head *bk_array;
	unsigned int power, max_load, min_load, tid;
};

/**
 * @anchor:	anchor point in struct HXhmap_node
 * @key:	data that works as key
 * @data:	data that works as value
 */
struct HXhmap_node {
	struct HXlist_head anchor;
	/* HXmap_node */
	union {
		void *key;
		const char *const skey;
	};
	union {
		void *data;
		char *sdata;
	};
};

struct HXmap_trav {
	enum HXmap_type type;
};

struct HXhmap_trav {
	struct HXmap_trav super;
	const struct HXhmap *hmap;
	const struct HXlist_head *head;
	unsigned int bk_current, tid;
};

/*
 * http://planetmath.org/encyclopedia/GoodHashTablePrimes.html
 * 23 and 3221.. added by j.eng.
 */
static const unsigned int HXhash_primes[] = {
	23, 53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157,
	98317, 196613, 393241, 786433, 1572869, 3145739, 6291469, 12582917,
	25165843, 50331653, 100663319, 201326611, 402653189, 805306457,
	1610612741, 3221225473U,
};

static void HXhmap_free(struct HXhmap *hmap)
{
	struct HXhmap_node *drop, *dnext;
	unsigned int i;

	for (i = 0; i < HXhash_primes[hmap->power]; ++i) {
		HXlist_for_each_entry_safe(drop, dnext,
		    &hmap->bk_array[i], anchor) {
			if (hmap->super.ops.k_free != NULL)
				hmap->super.ops.k_free(drop->key);
			if (hmap->super.ops.d_free != NULL)
				hmap->super.ops.d_free(drop->data);
			free(drop);
		}
	}

	free(hmap->bk_array);
	free(hmap);
}

EXPORT_SYMBOL void HXmap_free(struct HXmap *xmap)
{
	void *vmap = xmap;
	const struct HXmap_private *map = vmap;

	switch (map->type) {
	case HX_MAPTYPE_HASH:
		return HXhmap_free(vmap);
	default:
		break;
	}
}

static int HXmap_valuecmp(const void *pa, const void *pb, size_t len)
{
	return pa - pb;
}

static void *HXmap_valuecpy(const void *p, size_t len)
{
	return const_cast1(void *, p);
}

#define jrot(x,k) (((x) << (k)) | ((x) >> (32 - (k))))

/* jhash_mix - mix 3 32-bit values reversibly. */
#define jhash_mix(a, b, c) { \
	a -= c; a ^= jrot(c,  4); c += b; \
	b -= a; b ^= jrot(a,  6); a += c; \
	c -= b; c ^= jrot(b,  8); b += a; \
	a -= c; a ^= jrot(c, 16); c += b; \
	b -= a; b ^= jrot(a, 19); a += c; \
	c -= b; c ^= jrot(b,  4); b += a; \
}

#define jhash_final(a, b, c) { \
	c ^= b; c -= jrot(b, 14); \
	a ^= c; a -= jrot(c, 11); \
	b ^= a; b -= jrot(a, 25); \
	c ^= b; c -= jrot(b, 16); \
	a ^= c; a -= jrot(c,  4);  \
	b ^= a; b -= jrot(a, 14); \
	c ^= b; c -= jrot(b, 24); \
}

static unsigned long HXhash_jlookup3(const void *vkey, size_t length)
{
	static const unsigned int JHASH_GOLDEN_RATIO = 0x9e3779b9;
	const uint8_t *key = vkey;
	uint32_t a, b, c;

	a = b = c = JHASH_GOLDEN_RATIO + length;
	/* All but the last block: affect some 32 bits of (a,b,c) */
	for (; length > 12; length -= 12, key += 12) {
		a += key[0] + ((uint32_t)key[1] << 8) +
		     ((uint32_t)key[2] << 16) + ((uint32_t)key[3] << 24);
		b += key[4] + ((uint32_t)key[5] << 8) +
		     ((uint32_t)key[6] << 16) + ((uint32_t)key[7] << 24);
		c += key[8] + ((uint32_t)key[9] << 8) +
		     ((uint32_t)key[10] << 16)+ ((uint32_t)key[11] << 24);
		jhash_mix(a, b, c);
	}

	switch (length) {
	case 12: c += ((uint32_t)key[11]) << 24;
	case 11: c += ((uint32_t)key[10]) << 16;
	case 10: c += ((uint32_t)key[9])  << 8;
	case  9: c += key[8];
	case  8: b += ((uint32_t)key[7]) << 24;
	case  7: b += ((uint32_t)key[6]) << 16;
	case  6: b += ((uint32_t)key[5]) << 8;
	case  5: b += key[4];
	case  4: a += ((uint32_t)key[3]) << 24;
	case  3: a += ((uint32_t)key[2]) << 16;
	case  2: a += ((uint32_t)key[1]) << 8;
	case  1: a += key[0];
		break;
	case  0: return c;
	}

	jhash_final(a,b,c);
	return c;
}

static unsigned long HXhash_jlookup3v(const void *p, size_t z)
{
	return HXhash_jlookup3(&p, sizeof(p));
}

static unsigned long HXhash_djb2(const void *p, size_t z)
{
	const char *c = p;
	unsigned long v = 5381;

	while (*c != '\0')
		v = ((v << 5) + v) ^ *c++;
		/* v = v * 33 ^ *c++; */

	return v;
}

/**
 * Set up the operations for a map based on flags, and then override with
 * user-specified functions.
 */
static void HXmap_ops_setup(struct HXmap_private *super,
    const struct HXmap_ops *new_ops, unsigned int flags)
{
	struct HXmap_ops *ops = &super->ops;

	ops->k_clone = HXmap_valuecpy;
	ops->d_clone = HXmap_valuecpy;

	if (flags & HXMAP_SKEY)
		ops->k_compare = static_cast(void *, strcmp);
	else if (super->key_size == 0)
		ops->k_compare = HXmap_valuecmp;
	else
		ops->k_compare = memcmp;

	if (flags & HXMAP_CKEY) {
		ops->k_clone = (flags & HXMAP_SKEY) ?
		               static_cast(void *, HX_strdup) : HX_memdup;
		ops->k_free  = free;
	}
	if (flags & HXMAP_CDATA) {
		ops->d_clone = (flags & HXMAP_SDATA) ?
		               static_cast(void *, HX_strdup) : HX_memdup;
		ops->d_free  = free;
	}

	if (super->type == HX_MAPTYPE_HASH) {
		if (flags & HXMAP_SKEY)
			ops->k_hash = HXhash_djb2;
		else if (super->key_size != 0)
			ops->k_hash = HXhash_jlookup3;
		else
			ops->k_hash = HXhash_jlookup3v;
	}

	if (new_ops == NULL)
		return;

	/* Update with user-supplied functions */
	if (new_ops->k_compare != NULL)
		ops->k_compare = new_ops->k_compare;
	if (new_ops->k_clone != NULL)
		ops->k_clone   = new_ops->k_clone;
	if (new_ops->k_free != NULL)
		ops->k_free    = new_ops->k_free;
	if (new_ops->d_clone != NULL)
		ops->d_clone   = new_ops->d_clone;
	if (new_ops->d_free != NULL)
		ops->d_free    = new_ops->d_free;
	if (super->type == HX_MAPTYPE_HASH && new_ops->k_hash != NULL)
		ops->k_hash    = new_ops->k_hash;
}

/**
 * @n:	nominator of fraction
 * @d:	denominator of fraction
 * @v:	value
 *
 * Calculates @v * (@n / @d) without floating point or risk of overflow
 * (when @n <= @d).
 */
static inline unsigned int x_frac(unsigned int n, unsigned int d,
    unsigned int v)
{
	return (v / d) * n + (v % d) * n / d;
}

/**
 * HXhmap_move - move elements from one map to another
 * @bk_array:	target bucket array
 * @bk_number:	number of buckets
 * @hmap:	old hash table
 */
static void HXhmap_move(struct HXlist_head *bk_array, unsigned int bk_number,
    struct HXhmap *hmap)
{
	struct HXhmap_node *drop, *dnext;
	unsigned int bk_idx, i;

	for (i = 0; i < HXhash_primes[hmap->power]; ++i)
		HXlist_for_each_entry_safe(drop, dnext,
		    &hmap->bk_array[i], anchor) {
			bk_idx = hmap->super.ops.k_hash(drop->key,
			         hmap->super.key_size) % bk_number;
			HXlist_del(&drop->anchor);
			HXlist_add_tail(&bk_array[bk_idx], &drop->anchor);
		}
}

/**
 * HXhmap_layout - resize and rehash table
 * @hmap:	hash map
 * @prime_idx:	requested new table size (prime power thereof)
 */
static int HXhmap_layout(struct HXhmap *hmap, unsigned int power)
{
	const unsigned int bk_number = HXhash_primes[power];
	struct HXlist_head *bk_array, *old_array = NULL;
	unsigned int i;

	bk_array = malloc(bk_number * sizeof(*bk_array));
	if (bk_array == NULL)
		return -errno;
	for (i = 0; i < bk_number; ++i)
		HXlist_init(&bk_array[i]);
	if (hmap->bk_array != NULL) {
		HXhmap_move(bk_array, bk_number, hmap);
		old_array = hmap->bk_array;
	}
	hmap->power    = power;
	hmap->min_load = (power != 0) ? HXhash_primes[power] / 4 : 0;
	hmap->max_load = x_frac(7, 10, HXhash_primes[power]);
	hmap->bk_array = bk_array;
	++hmap->tid;
	free(old_array);
	return 1;
}

EXPORT_SYMBOL struct HXmap *HXhashmap_init4(unsigned int flags,
    const struct HXmap_ops *ops, size_t key_size, size_t data_size)
{
	struct HXmap_private *super;
	struct HXhmap *hmap;
	int saved_errno;

	if ((hmap = calloc(1, sizeof(*hmap))) == NULL)
		return NULL;

	super            = &hmap->super;
	super->flags     = flags;
	super->items     = 0;
	super->type      = HX_MAPTYPE_HASH;
	super->key_size  = key_size;
	super->data_size = data_size;
	HXmap_ops_setup(super, ops, flags);
	hmap->tid = 1;
	errno = HXhmap_layout(hmap, 0);
	if (hmap->bk_array == NULL)
		goto out;

	errno = 0;
	return static_cast(void *, hmap);

 out:
	saved_errno = errno;
	HXhmap_free(hmap);
	errno = saved_errno;
	return NULL;
}

EXPORT_SYMBOL struct HXmap *HXhashmap_init(unsigned int flags)
{
	return HXhashmap_init4(flags, NULL, 0, 0);
}

static struct HXhmap_node *
HXhmap_find(const struct HXhmap *hmap, const void *key)
{
	struct HXhmap_node *drop;
	unsigned int bk_idx;

	bk_idx = hmap->super.ops.k_hash(key, hmap->super.key_size) %
	         HXhash_primes[hmap->power];
	HXlist_for_each_entry(drop, &hmap->bk_array[bk_idx], anchor)
		if (hmap->super.ops.k_compare(key, drop->key,
		    hmap->super.key_size) == 0)
			return drop;
	return NULL;
}

EXPORT_SYMBOL void *HXmap_get(const struct HXmap *xmap, const void *key)
{
	const void *vmap = xmap;
	const struct HXmap_private *map = vmap;

	switch (map->type) {
	case HX_MAPTYPE_HASH: {
		const struct HXhmap_node *drop = HXhmap_find(vmap, key);
		if (drop != NULL) {
			errno = 0;
			return drop->data;
		}
		errno = -ENOENT;
		return NULL;
	}
	default:
		errno = EINVAL;
		return NULL;
	}
}

/**
 * HXhmap_replace - replace value in a drop
 */
static int HXhmap_replace(const struct HXhmap *hmap, struct HXhmap_node *drop,
    const void *value)
{
	void *old_value, *new_value;

	if (hmap->super.flags & HXMAP_NOREPLACE)
		return -EEXIST;

	new_value = hmap->super.ops.d_clone(value, hmap->super.data_size);
	if (new_value == NULL && value != NULL)
		return -errno;
	old_value  = drop->data;
	drop->data = new_value;
	if (hmap->super.ops.d_free != NULL)
		hmap->super.ops.d_free(old_value);
	return 1;
}

static int HXhmap_add(struct HXhmap *hmap, const void *key, const void *value)
{
	struct HXhmap_node *drop;
	unsigned int bk_idx;
	int ret, saved_errno;

	if ((drop = HXhmap_find(hmap, key)) != NULL)
		return HXhmap_replace(hmap, drop, value);

	if (hmap->super.items >= hmap->max_load &&
	    hmap->power < ARRAY_SIZE(HXhash_primes) - 1) {
		if ((ret = HXhmap_layout(hmap, hmap->power + 1)) <= 0)
			return ret;
	} else if (hmap->super.items < hmap->min_load && hmap->power > 0) {
		if ((ret = HXhmap_layout(hmap, hmap->power - 1)) <= 0)
			return ret;
	}

	/* New node */
	if ((drop = malloc(sizeof(*drop))) == NULL)
		return -errno;
	HXlist_init(&drop->anchor);
	drop->key = hmap->super.ops.k_clone(key, hmap->super.key_size);
	if (drop->key == NULL)
		goto out;
	drop->data = hmap->super.ops.d_clone(value, hmap->super.data_size);
	if (drop->data == NULL && value != NULL)
		goto out;

	bk_idx = hmap->super.ops.k_hash(key, hmap->super.key_size) %
	         HXhash_primes[hmap->power];
	HXlist_add_tail(&hmap->bk_array[bk_idx], &drop->anchor);
	++hmap->super.items;
	return 1;

 out:
	saved_errno = errno;
	if (hmap->super.ops.k_free != NULL)
		hmap->super.ops.k_free(drop->key);
	free(drop);
	return -(errno = saved_errno);
}

EXPORT_SYMBOL int HXmap_add(struct HXmap *xmap,
    const void *key, const void *value)
{
	void *vmap = xmap;
	struct HXmap_private *map = vmap;

	switch (map->type) {
	case HX_MAPTYPE_HASH:
		return HXhmap_add(vmap, key, value);
	default:
		return -EINVAL;
	}
}

static void *HXhmap_del(struct HXhmap *hmap, const void *key)
{
	struct HXhmap_node *drop;
	void *value;

	if ((drop = HXhmap_find(hmap, key)) == NULL) {
		errno = ENOENT;
		return NULL;
	}

	HXlist_del(&drop->anchor);
	++hmap->tid;
	--hmap->super.items;
	if (hmap->super.items < hmap->min_load && hmap->power > 0)
		/*
		 * Ignore return value. If it failed, it will continue to use
		 * the current bk_array.
		 */
		HXhmap_layout(hmap, hmap->power - 1);

	value = drop->data;
	if (hmap->super.ops.k_free != NULL)
		hmap->super.ops.k_free(drop->key);
	if (hmap->super.ops.d_free != NULL)
		hmap->super.ops.d_free(drop->data);
	free(drop);
	errno = 0;
	return value;
}

EXPORT_SYMBOL void *HXmap_del(struct HXmap *xmap, const void *key)
{
	void *vmap = xmap;
	struct HXmap_private *map = vmap;

	switch (map->type) {
	case HX_MAPTYPE_HASH:
		return HXhmap_del(vmap, key);
	default:
		errno = EINVAL;
		return NULL;
	}
}

static void *HXhmap_travinit(const struct HXhmap *hmap)
{
	struct HXhmap_trav *trav;

	if ((trav = malloc(sizeof(*trav))) == NULL)
		return NULL;
	trav->super.type = HX_MAPTYPE_HASH;
	trav->hmap = hmap;
	trav->head = NULL;
	trav->bk_current = 0;
	trav->tid = hmap->tid;
	return trav;
}

EXPORT_SYMBOL void *HXmap_travinit(const struct HXmap *xmap)
{
	const void *vmap = xmap;
	const struct HXmap_private *map = vmap;

	switch (map->type) {
	case HX_MAPTYPE_HASH:
		return HXhmap_travinit(vmap);
	default:
		errno = EINVAL;
		return NULL;
	}
}

static const struct HXmap_node *HXhmap_traverse(void *xtrav)
{
	struct HXhmap_trav *trav  = xtrav;
	const struct HXhmap *hmap = trav->hmap;
	const struct HXhmap_node *drop;

	if (trav->tid != hmap->tid)
		/*
		 * Hashmap changed / elements may have a completely new order,
		 * stop traversing.
		 */
		return NULL;

	if (trav->head == NULL)
		trav->head = hmap->bk_array[trav->bk_current].next;
	else
		trav->head = trav->head->next;

	while (trav->head == &hmap->bk_array[trav->bk_current]) {
		if (++trav->bk_current >= HXhash_primes[hmap->power])
			return false;
		trav->head = hmap->bk_array[trav->bk_current].next;
	}

	drop = HXlist_entry(trav->head, struct HXhmap_node, anchor);
	return static_cast(const void *, &drop->key);
}

EXPORT_SYMBOL const struct HXmap_node *HXmap_traverse(void *xtrav)
{
	const struct HXmap_trav *trav = xtrav;

	if (xtrav == NULL)
		return NULL;

	switch (trav->type) {
	case HX_MAPTYPE_HASH:
		return HXhmap_traverse(xtrav);
	default:
		errno = EINVAL;
		return NULL;
	}
}

EXPORT_SYMBOL void HXmap_travfree(void *xtrav)
{
	if (xtrav == NULL)
		return;
	/*
	 * All implementations have no further nested allocated data
	 * at this time.
	 */
	free(xtrav);
}
