#ifndef _LIBHX_ARBTREE_H
#define _LIBHX_ARBTREE_H 1

#ifdef __cplusplus
extern "C" {
#endif

enum {
	/* activates key=>value pairs */
	HXBT_MAP   = 1 << 0,
	/* copy key  (only for string keys!) */
	HXBT_CKEY  = 1 << 1,
	/* copy data (only for string data!) */
	HXBT_CDATA = 1 << 2,
	/* pointer to comparison routine passed */
	HXBT_CMPFN = 1 << 3,
	/* use direct integer comparison */
	HXBT_ICMP  = 1 << 4,
	/* use strcmp() -- abbreviation for HXBT_CMPFN,strcmp */
	HXBT_SCMP  = 1 << 5,
	HXBT_SKEY  = 1 << 5, /* init2 */
	/* use CIDs for traverser */
	HXBT_CID   = 1 << 6,
	HXBT_SDATA = 1 << 7, /* init2 */
};

struct HXbtree_node {
	struct HXbtree_node *sub[2];
	union {
		void *key;
		const char *const skey;
	};
	union {
		void *data;
		char *sdata;
	};
	unsigned char color;
};

struct HXbtree {
	int (*k_compare)(const void *, const void *, size_t);
	void *uptr;
	struct HXbtree_node *root;
	unsigned int items, tid;
	unsigned char flags;
	void *(*k_clone)(const void *, size_t);
	void (*k_free)(void *);
	void *(*d_clone)(const void *, size_t);
	void (*d_free)(void *);
	size_t key_size, data_size;
};

extern struct HXbtree *HXbtree_init(unsigned int, ...);
extern struct HXbtree *HXbtree_init2(unsigned int,
	int (*)(const void *, const void *, size_t),
	void *(*)(const void *, size_t), void (*)(void *),
	void *(*)(const void *, size_t), void (*)(void *),
	size_t, size_t);
extern struct HXbtree_node *HXbtree_add(struct HXbtree *, const void *, ...);
extern struct HXbtree_node *HXbtree_find(const struct HXbtree *, const void *);
extern void *HXbtree_get(const struct HXbtree *, const void *);
extern void *HXbtree_del(struct HXbtree *, const void *);
extern void HXbtree_free(struct HXbtree *);
extern void *HXbtrav_init(const struct HXbtree *);
extern struct HXbtree_node *HXbtraverse(void *);
extern void HXbtrav_free(void *);

#ifdef __cplusplus
} /* extern "C" */

extern "C++" {

template<typename type> static inline type
HXbtree_get(struct HXbtree *bt, const void *ptr)
{
	return reinterpret_cast<type>(HXbtree_get(bt, ptr));
}

template<typename type> static inline type
HXbtree_del(struct HXbtree *bt, const void *ptr)
{
	return reinterpret_cast<type>(HXbtree_del(bt, ptr));
}

} /* extern "C++" */
#endif

#endif /* _LIBHX_ARBTREE_H */
