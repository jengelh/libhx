#ifndef __cplusplus
#	include <stdlib.h>
#else
#	include <cstdlib>
#endif
#include <libHX/option.h>

static const struct HXoption t[] = {
	HXOPT_AUTOHELP,
	HXOPT_TABLEEND,
};

int main(int argc, const char **argv)
{
	HX_getopt(t, &argc, &argv, HXOPT_USAGEONERR);
	return EXIT_SUCCESS;
}
