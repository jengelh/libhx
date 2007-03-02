#include <stdio.h>
#include <libHX.h>

int main(void)
{
	void *d = HXdir_open("/tmp");
	char *n;

	printf("Available files in /tmp:\n");
	while((n = HXdir_read(d)) != NULL)
		printf("\t" "%s\n", n);

	HXdir_close(d);
	return 0;
}
