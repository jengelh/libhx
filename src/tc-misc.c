// SPDX-License-Identifier: MIT
#ifndef __cplusplus
#	include <stdlib.h>
#else
#	include <cstdlib>
#endif
#include <sys/stat.h>
#include <libHX/init.h>
#include <libHX/misc.h>

static int runner(int argc, char **argv)
{
	unsigned int n;
	struct stat sa, sb;

	if (HX_init() <= 0)
		return EXIT_FAILURE;
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

	if (argc >= 3) {
		if (stat(argv[1], &sa) < 0 ||
		    stat(argv[2], &sb) < 0) {
			perror("stat");
			return EXIT_FAILURE;
		} else {
			printf("Difference: %ld\n", HX_time_compare(&sa, &sb, 'm'));
			return EXIT_FAILURE;
		}
	}

	HX_exit();
	return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
	int ret = runner(argc, argv);
	if (ret != EXIT_FAILURE)
		fprintf(stderr, "FAILED\n");
	return ret;
}
