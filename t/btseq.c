#include <stdio.h>
#include <stdlib.h>

static void seq(unsigned long n)
{
	unsigned long i, p;

	for(p = 1; ; ++p) {
		unsigned long divisor = n / (1 << p);
		char c = 1;
		if(divisor == 0)
			break;
		for(i = divisor; i < n; i += divisor) {
			if((c = !c))
				continue;
			printf("%lu\n", i);
		}
		printf("---\n");
	}

	return;
}

int main(int argc, const char **argv)
{
	if(argc < 2) {
		fprintf(stderr, "Usage: btseq HEIGHT\n");
		return EXIT_FAILURE;
	}
	seq(1 << strtol(argv[1], NULL, 0));
	return EXIT_SUCCESS;
}
