/*
 *	Copyright Jan Engelhardt
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the WTF Public License version 2 or
 *	(at your option) any later version.
 */
#ifdef __cplusplus
#	include <cstdio>
#	include <cstdlib>
#else
#	include <stdbool.h>
#	include <stdio.h>
#	include <stdlib.h>
#endif
#include <libHX/list.h>
#include <libHX/init.h>
#include <libHX/misc.h>

struct text_object {
	struct HXlist_head list;
	char id[5];
};

union list_encap {
	struct HXlist_head list;
};

static HXCLIST_HEAD(strings_ct);

static void l_init(unsigned int max, bool unshift)
{
	static const char *const msg[] = {"Pushing", "Unshifting"};
	struct text_object *obj;
	unsigned int i;

	for (i = 1; i <= max; ++i) {
#ifdef __cplusplus
		obj = new struct text_object;
#else
		obj = malloc(sizeof(*obj));
#endif
		HXlist_init(&obj->list);
		obj->id[0] = HX_irand('a', 'z'+1);
		obj->id[1] = HX_irand('a', 'z'+1);
		obj->id[2] = HX_irand('a', 'z'+1);
		obj->id[3] = HX_irand('a', 'z'+1);
		obj->id[4] = '\0';
		printf("%s item %u (\"%s\")\n", msg[unshift], i, obj->id);
		if (unshift)
			HXclist_unshift(&strings_ct, &obj->list);
		else
			HXclist_push(&strings_ct, &obj->list);
	}
}

static void l_traverse(void)
{
	const struct text_object *obj, *safe;
	unsigned int i;

	i = 0;
	HXlist_for_each_entry_safe(obj, safe, &strings_ct, list)
		printf("Retrieving item %u (\"%s\")\n", ++i, obj->id);
}

static void l_dump(bool pop)
{
	static const char *const msg[] = {"Shifting", "Popping"};
	struct text_object *obj;
	unsigned int i = 0;

	while ((obj = (pop ?
	    HXclist_pop(&strings_ct, struct text_object, list) :
	    HXclist_shift(&strings_ct, struct text_object, list)
	    )) != NULL)
		printf("%s item %u (\"%s\")\n", msg[pop], ++i, obj->id);

	printf("Remaining elements: %u\n", strings_ct.items);
}

static void l_empty(void)
{
	static const char *const fstr[] = {"fail", "pass"};
	struct HXclist_head clh;
	struct HXlist_head lh;
	union list_encap *pos;
	unsigned int count = 0;
	bool success = true;

	HXlist_init(&lh);
	HXclist_init(&clh);

	HXlist_for_each_entry(pos, &lh, list)
		success = false;
	HXlist_for_each_entry(pos, &clh, list)
		success = false;

	printf("Zero traversal: %s\n", fstr[success]);
	printf("The list is indeed%s empty\n",
	       HXlist_empty(&lh) ? "" : " NOT");

	HXclist_push(&clh, &lh);
	HXlist_for_each_entry(pos, &clh, list)
		++count;

	printf("One traversal: %s\n", fstr[count == 1]);
	printf("The list is indeed%s empty\n",
	       HXlist_empty(&lh) ? "" : " NOT");
}

static void l_shift(void)
{
	/* Check for -Wshadow warnings */
	struct object {
		int value;
		struct HXlist_head anchor;
	};
	HXCLIST_HEAD(clh);
	struct object q, *p;
	void *x;

	q.value = 1337;
	HXlist_init(&q.anchor);
	HXclist_push(&clh, &q.anchor);
	x = p = HXclist_shift(&clh, struct object, anchor);
	printf("%d\n", p->value);

	HXlist_init(&q.anchor);
	HXclist_push(&clh, &q.anchor);
	x = p = HXclist_pop(&clh, struct object, anchor);
	printf("%d\n", p->value);
}

int main(int argc, const char **argv)
{
	unsigned int max = 10;

	if (HX_init() <= 0)
		abort();
	if (argc >= 2)
		max = strtoul(argv[1], NULL, 0);

	l_init(max, HX_rand() & 1);
	l_traverse();
	l_dump(HX_rand() & 1);
	l_empty();
	l_shift();
	HX_exit();
	return EXIT_SUCCESS;
}
