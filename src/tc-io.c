#include <stdio.h>
#include <libHX/io.h>

int main(void)
{
	size_t z;
	char *s = HX_slurp_file("tc-io.c", &z);
	printf("%s\n", s);
	printf("Dumped %zu bytes\n", z);
	return 0;
}
