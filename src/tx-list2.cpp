#include <stdio.h>
#include <stdlib.h>
#include <libHX/list.h>

struct item {
	struct HXlist_head list;
};

int main(void)
{
	HXLIST_HEAD(clh);
	HXLIST_HEAD(lh);
	struct item *pos;

	HXlist_add_tail(&clh, &lh);
	HXlist_for_each_entry(pos, &clh, list)
#ifdef OK
		printf("%p\n", pos);
#else
		;
#endif
	return EXIT_SUCCESS;
}
