#ifndef __cplusplus
#	include <stdlib.h>
#else
#	include <cstdlib>
#endif
#include <libHX.h>

int main(void)
{
	unsigned int n;

	printf("%d\n", HX_ffs(0));
	for (n = 1; ; n <<= 1) {
		printf("%08x = %d\n", n, HX_ffs(n));
		if (n & 0x80000000)
			break;
	}
	printf("---\n");
	for (n = 1; ; n <<= 1, n |= 1) {
		printf("%08x = %d\n", n, HX_ffs(n));
		if (n == ~0U)
			break;
	}

	return EXIT_SUCCESS;
}
