// SPDX-License-Identifier: MIT
/*
 *	Behavior Correctness Test for HX_strchr2
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
