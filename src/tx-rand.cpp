/*
 *	this program is released in the Public Domain
 */
#include <stdio.h>
#include <stdlib.h>
#include <libHX/misc.h>

int main(void)
{
	unsigned int i;

	for (i = 0; i < 15; ++i)
		printf("%08x ", HX_irand(0, RAND_MAX));
	printf("\n");

	return EXIT_SUCCESS;
}
