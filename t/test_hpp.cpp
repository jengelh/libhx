#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <libHX.h>

int main(void)
{
	struct HXbtree *bt;
	const char *tmp;
	bt = HXbtree_init(HXBT_MAP | HXBT_CKEY | HXBT_CDATA | HXBT_SCMP);
	HXbtree_add(bt, "foo", "bar");
	tmp = HXbtree_get<const char *>(bt, "foo");
	printf("%s\n", tmp);
	HXbtree_free(bt);

	void *hnd = HX_dlopen("/lib/libm.so.6");
	if(hnd != NULL) {
		double (*cosp)(double) = HX_dlsym<double (*)(double)>(hnd, "cos");
		printf("%f\n", (*cosp)(2.0));
		HX_dlclose(hnd);
	}
	return EXIT_SUCCESS;
}
