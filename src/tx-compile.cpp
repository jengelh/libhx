/* This file is for testing the cumulative include */
#ifndef __cplusplus
#	include <stdlib.h>
#else
#	include <cstdlib>
#endif
#include <libHX.h>
#include <libHX/defs.h>
#include <libHX/list.h>
#include <libHX/misc.h>
#include <libHX/ctype_helper.h>

#define ZZ 64

int main(void)
{
	unsigned long bitmap[HXbitmap_size(unsigned long, 64)];

	printf("sizeof bitmap: %zu, array_size: %zu\n",
	       sizeof(bitmap), ARRAY_SIZE(bitmap));
	HXbitmap_set(bitmap, 0);
	printf(HX_STRINGIFY(1234+2 +2) "," HX_STRINGIFY(ZZ) "\n");
	return EXIT_SUCCESS;
}
