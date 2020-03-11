/*
 *	Copyright Jan Engelhardt
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the WTF Public License version 2 or
 *	(at your option) any later version.
 */
#ifndef __cplusplus
#	include <stdio.h>
#	include <stdlib.h>
#else
#	include <cstdio>
#	include <cstdlib>
#endif
#include <libHX/init.h>
#include <libHX/io.h>
#include <libHX/misc.h>

static void lookatdir(const char *dname)
{
	struct HXdir *dh;
	const char *n;

	dh = HXdir_open(dname);
	printf("Available files in %s:\n", dname);
	while ((n = HXdir_read(dh)) != NULL)
		printf("\t" "%s\n", n);
	HXdir_close(dh);
}

int main(int argc, const char **argv)
{
	if (HX_init() <= 0)
		abort();
	if (argc == 1) {
		/* On Windows VCRT, "/" yields nothing, "c:/" is needed */
		lookatdir("/");
		lookatdir("c:/");
		lookatdir(".");
	} else {
		while (*++argv != NULL)
			lookatdir(*argv);
	}
	HX_exit();
	return EXIT_SUCCESS;
}
