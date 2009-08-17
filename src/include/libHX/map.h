#ifndef _LIBHX_MAP_H
#define _LIBHX_MAP_H 1

#include <sys/types.h>
#ifndef __cplusplus
#	include <stdbool.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Flags changable at runtime:
 * %HXMAP_NOREPLACE:	Calling HXmap_add() for an already existing key will
 * 			throw an error (no-overwrite semantics)
 *
 * Initialization-time flags only:
 * %HXMAP_SKEY:		Key will be a C-style string (sets ops->k_*)
 * %HXMAP_CKEY:		Make a copy of the key on HXmap_add
 * %HXMAP_SDATA:	Data will be a C-style string (presets ops->d_*)
 * %HXMAP_CDATA:	Make a copy of the data on HXmap_add
 */
enum {
	HXMAP_NOREPLACE = 1 << 0,
	HXMAP_SKEY      = 1 << 28,
	HXMAP_CKEY      = 1 << 29,
	HXMAP_SDATA     = 1 << 30,
	HXMAP_CDATA     = 1 << 31,

	HXMAP_SCKEY     = HXMAP_SKEY | HXMAP_CKEY,
	HXMAP_SCDATA    = HXMAP_SDATA | HXMAP_CDATA,
};

struct HXhmap;

/**
 * @items:	number of items in the map
 * @flags:	flags for this map
 */
struct HXmap {
	unsigned int items, flags;
};

struct HXmap_ops {
	/* k_compare: the size argument is needed for memcmp. */
	int (*k_compare)(const void *, const void *, size_t);
	void *(*k_clone)(const void *, size_t);
	void (*k_free)(void *);
	void *(*d_clone)(const void *, size_t);
	void (*d_free)(void *);
	unsigned long (*k_hash)(const void *, size_t);
};

struct HXmap_node {
	union {
		void *key;
		const char *const skey;
	};
	union {
		void *data;
		char *sdata;
	};
};

extern struct HXmap *HXhashmap_init(unsigned int);
extern struct HXmap *HXhashmap_init4(unsigned int, const struct HXmap_ops *,
	size_t, size_t);

extern int HXmap_add(struct HXmap *, const void *, const void *);
extern void *HXmap_get(const struct HXmap *, const void *);
extern void *HXmap_travinit(const struct HXmap *);
extern const struct HXmap_node *HXmap_traverse(void *);
extern void HXmap_travfree(void *);
extern void HXmap_free(struct HXmap *);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _LIBHX_MAP_H */
