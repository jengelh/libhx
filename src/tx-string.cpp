/* long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <libHX/string.h>
#include <libHX.h>

static void t_path(void)
{
	printf("# dirname\n");
	printf("%s\n", HX_dirname("/"));
	printf("%s\n", HX_dirname("//"));
	printf("%s\n", HX_dirname("etc//foo/"));
	printf("%s\n", HX_dirname("//etc//foo//"));
	printf("# basename\n");
	printf("%s\n", HX_basename("/"));
	printf("%s\n", HX_basename("/."));
	printf("%s\n", HX_basename("//"));
	printf("%s\n", HX_basename("etc/foo"));
	printf("%s\n", HX_basename("etc//foo"));
	printf("%s\n", HX_basename("//etc//foo"));
}

static void t_strncat(void)
{
	char data[5] = "DATA";

	if (snprintf(data, sizeof(data), "12345678") >= sizeof(data))
		printf("Not enoguh space\n");
	printf("String: >%s<\n", data);

	HX_strlcat(data, "pqrstuv__", 2);
	printf("String: >%s<\n", data);

	*data = '\0';
	HX_strlcat(data, "123456789", sizeof(data));
	printf("String: >%s<\n", data);

	*data = '\0';
	HX_strlncat(data, "123456789", sizeof(data), 9);
	printf("String: >%s<\n", data);
}

static void t_strsep(void)
{
	char b[] = "jengelh:x:1500:100:Jan Engelhardt:/home/jengelh:/bin/bash";
	char *wp = b, *ret;

	printf("# strsep\n");
	while ((ret = HX_strsep2(&wp, ":")) != NULL)
		printf("%s\n", ret);
}

static void t_split(void)
{
	int fd;
	char **ar = HX_split(
		"root:x:0:0:Jan Engelhardt:/home/jengelha:/bin/bash", ":", &fd, 0);
	/* Analyze @ar with debugger */
	HX_zvecfree(ar);
}

int main(int argc, const char **argv)
{
	hmc_t *tx = NULL;
	const char *file = (argc >= 2) ? argv[1] : "t-string.c";
	FILE *fp = fopen(file, "r");

	if (fp == NULL) {
		fprintf(stderr, "Cannot open %s: %s\n", file, strerror(errno));
	} else {
		while (HX_getl(&tx, fp) != NULL)
			printf("%s", tx);
		fclose(fp);
	}

	t_path();
	t_strncat();
	t_strsep();
	t_split();
	return EXIT_SUCCESS;
}
