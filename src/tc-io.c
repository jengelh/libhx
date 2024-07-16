// SPDX-License-Identifier: MIT
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libHX/io.h>
#include "internal.h"

static void sf(void)
{
	int src = open("tc-io.c", O_RDONLY);
	if (src < 0)
		return;
	int dst = open("/dev/null", O_WRONLY);
	if (dst < 0) {
		close(src);
		return;
	}
	ssize_t ret = HX_sendfile(dst, src, SIZE_MAX);
	if (ret < 0)
		printf("sendfile: %s\n", strerror(errno));
	else
		printf("sendfile transferred %zd bytes\n", ret);
	close(dst);
	close(src);
}

static void t_getcwd(void)
{
	hxmc_t *s = nullptr;
	if (HX_getcwd(&s) > 0)
		printf("cwd1: >%s<\n", s);
	HXmc_setlen(&s, 0);
	if (HX_getcwd(&s) > 0)
		printf("cwd2: >%s<\n", s);
}

int main(void)
{
	size_t z;
	char *s = HX_slurp_file("tc-io.c", &z);
	if (s == nullptr) {
		fprintf(stderr, "HX_slurp_file: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}
	printf("%s\n", s);
	printf("Dumped %zu bytes\n", z);
	free(s);
	s = HX_slurp_file("/proc/version", &z);
	if (s == nullptr) {
		fprintf(stderr, "HX_slurp_file: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}
	printf(">%s<\n", s);
	free(s);

	sf();
	int ret = HX_copy_file("tc-io.c", "tciocopy.txt", 0);
	if (ret <= 0) {
		fprintf(stderr, "HX_copy_file: %s\n", strerror(errno));
	} else {
		fprintf(stderr, "copy_file ok\n");
		unlink("tciocopy.txt");
	}

	t_getcwd();
	return 0;
}
