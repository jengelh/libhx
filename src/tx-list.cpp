#ifdef __cplusplus
#	include <cstdio>
#	include <cstdlib>
#else
#	include <stdbool.h>
#	include <stdio.h>
#	include <stdlib.h>
#endif
#include <libHX/clist.h>
#include <libHX.h>

struct text_object {
	struct HXlist_head list;
	char id[5];
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
		HXlist_init_head(&obj->list);
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
	const struct text_object *obj;
	unsigned int i;

	i = 0;
	HXlist_for_each_entry(obj, &strings_ct, list)
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

int main(int argc, const char **argv)
{
	unsigned int max = 10;

	if (argc >= 2)
		max = strtoul(argv[1], NULL, 0);

	l_init(max, HX_rand() & 1);
	l_traverse();
	l_dump(HX_rand() & 1);
	return EXIT_SUCCESS;
}
