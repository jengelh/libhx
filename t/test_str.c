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
	return 0;
}

//=============================================================================
