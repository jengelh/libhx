#ifndef _LIBHX_MAP_H
#define _LIBHX_MAP_H 1

#ifdef __cplusplus
extern "C" {
#endif

struct HXmap_ops {
	/* k_compare: the size argument is needed for memcmp. */
	int (*k_compare)(const void *, const void *, size_t);
	void *(*k_clone)(const void *, size_t);
	void (*k_free)(void *);
	void *(*d_clone)(const void *, size_t);
	void (*d_free)(void *);
};

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _LIBHX_MAP_H */
