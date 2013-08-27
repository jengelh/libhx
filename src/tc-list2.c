/*
 *	Copyright Jan Engelhardt
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the WTF Public License version 2 or
 *	(at your option) any later version.
 */
#include <stdio.h>
#include <stdlib.h>
#include <libHX/list.h>

struct item {
	struct HXlist_head list;
};

int main(void)
{
	HXLIST_HEAD(clh);
	struct item lh;
	struct item *pos;

	HXlist_init(&lh.list);
	HXlist_add_tail(&clh, &lh.list);
	HXlist_for_each_entry(pos, &clh, list)
#ifdef OK
		printf("%p\n", pos);
#else
		;
#endif
	return EXIT_SUCCESS;
}
