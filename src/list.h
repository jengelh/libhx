#ifndef _LIBHX_LIST_H
#define _LIBHX_LIST_H 1

#ifdef __cplusplus
#	include <cstddef>
#else
#	include <stddef.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef container_of
#	define container_of(ptr, type, member) ({			\
		const typeof(((type *)0)->member) *__mptr = (ptr);	\
		(type *)((char *)__mptr - offsetof(type, member));	\
	})
#endif

struct HXlist_head {
	struct HXlist_head *next, *prev;
};

#define HXLIST_HEAD_INIT(name) {&(name), &(name)}
#define HXLIST_HEAD(name) \
	struct HXlist_head name = HXLIST_HEAD_INIT(name)

static inline void HXlist_init_head(struct HXlist_head *list)
{
	list->next = list->prev = list;
}

static inline void __HXlist_add(struct HXlist_head *nu,
    struct HXlist_head *prev, struct HXlist_head *next)
{
	nu->next = next;
	nu->prev = prev;
	next->prev = nu;
	prev->next = nu;
}

static inline void HXlist_add(struct HXlist_head *head, 
    struct HXlist_head *entry)
{
	__HXlist_add(entry, head, head->next);
}

static inline void HXlist_add_tail(struct HXlist_head *head,
    struct HXlist_head *entry)
{
	__HXlist_add(entry, head->prev, head);
}

static inline void HXlist_del(struct HXlist_head *entry)
{
	entry->prev->next = entry->next;
	entry->next->prev = entry->prev;
	entry->next = NULL;
	entry->prev = NULL;
}

#define HXlist_for_each(pos, head) \
	for ((pos) = (head)->next; (pos) != (void *)(head); \
	     (pos) = (pos)->next)

#define HXlist_for_each_entry(pos, head, member) \
	for ((pos) = container_of((head)->next, typeof(*(pos)), member); \
	     &(pos)->member != (void *)(head); \
	     (pos) = container_of((pos)->member.next, typeof(*(pos)), member))

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _LIBHX_LIST_H */
