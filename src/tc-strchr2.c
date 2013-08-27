/*
 *	Behavior Correctness Test for HX_strchr2
 *	Copyright Jan Engelhardt, 2013
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the WTF Public License version 2 or
 *	(at your option) any later version.
 */
#include <libHX/string.h>

static const char alphabet[] = "abcdefghijklmnopqrstuvwxyz";

int main(void)
{
	char *z;

	z = HX_strchr2("qfm!bar", alphabet);
	if (z == NULL || *z != '!')
		return EXIT_FAILURE;

	z = HX_strchr2("qfmxbar", alphabet);
	if (z != NULL)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
