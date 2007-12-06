#include <sys/types.h>
#include <stdio.h>
#include <libHX.h>

int main(void)
{
	int fd;
	char **ar = HX_split(
		"root:x:0:0:Jan Engelhardt:/home/jengelha:/bin/bash", ":", &fd, 0);
	/* Analyze @ar with debugger */
	HX_zvecfree(ar);

	char *str = "owned";
	int w = 2;
	printf("%.*s\n", w, str);

	char *tx = NULL;
	FILE *fp = fopen("longline.txt", "r");
	while (HX_getl(&tx, fp) != NULL)
		printf("%s", tx);

	fclose(fp);

	printf("%s\n", HX_dirname("/"));
	printf("%s\n", HX_dirname("//"));
	printf("%s\n", HX_dirname("etc//foo/"));
	printf("%s\n", HX_dirname("//etc//foo//"));
	return 0;
}

//=============================================================================
