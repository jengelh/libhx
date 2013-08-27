/*
A=b;C="d" ; E="F;" ; F= G=Z
*/
/*
 *	shconfig test program
 *	Copyright Jan Engelhardt
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the WTF Public License version 2 or
 *	(at your option) any later version.
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX/init.h>
#include <libHX/map.h>
#include <libHX/option.h>

static void t_shconfig(const char *file)
{
	char *A, *C, *E;
	struct HXoption opt_tab[] = {
		{.ln = "A", .type = HXTYPE_STRING, .ptr = &A},
		{.ln = "C", .type = HXTYPE_STRING, .ptr = &C},
		{.ln = "E", .type = HXTYPE_STRING, .ptr = &E},
		HXOPT_TABLEEND,
	};
	if (HX_shconfig(file, opt_tab) < 0)
		fprintf(stderr, "Read error %s: %s\n", file, strerror(errno));
}

static void t_shconfig2(const char *file)
{
	const struct HXmap_node *node;
	struct HXmap_trav *trav;
	struct HXmap *map;

	map = HX_shconfig_map(file);
	if (map == NULL) {
		fprintf(stderr, "HX_shconfig_map: %s\n", strerror(errno));
		abort();
	}
	trav = HXmap_travinit(map, HXMAP_NOFLAGS);
	while ((node = HXmap_traverse(trav)) != NULL)
		printf("\t\"%s\" -> \"%s\"\n", node->skey, node->sdata);
	HXmap_travfree(trav);
}

int main(int argc, const char **argv)
{
	int ret;

	ret = HX_init();
	if (ret <= 0) {
		fprintf(stderr, "HX_init: %s\n", strerror(-ret));
		return EXIT_FAILURE;
	}
	t_shconfig((argc >= 2) ? argv[1] : "tc-shconf.c");
	t_shconfig2((argc >= 2) ? argv[1] : "tc-shconf.c");
	HX_exit();
	return EXIT_SUCCESS;
}
