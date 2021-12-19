#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <libHX/io.h>

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
	printf("sendfile transferred %zd bytes\n", ret);
	close(dst);
	close(src);
}

int main(void)
{
	size_t z;
	char *s = HX_slurp_file("tc-io.c", &z);
	printf("%s\n", s);
	printf("Dumped %zu bytes\n", z);

	sf();
	int ret = HX_copy_file("tc-io.c", "tciocopy.txt", 0);
	if (ret <= 0)
		fprintf(stderr, "HX_copy_file: %s\n", strerror(errno));
	else
		fprintf(stderr, "copy_file ok\n");
	return 0;
}
