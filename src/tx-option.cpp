#ifndef __cplusplus
#	include <stdlib.h>
#else
#	include <cstdlib>
#endif
#include <libHX/option.h>

static unsigned int g_verbose;
static const struct HXoption t[] = {
	{nullptr, 'v', HXTYPE_NONE | HXOPT_INC, &g_verbose},
	HXOPT_AUTOHELP,
	HXOPT_TABLEEND,
};

int main(int argc, char **argv)
{
	if (HX_getopt(t, &argc, &argv, HXOPT_USAGEONERR) != HXOPT_ERR_SUCCESS)
		return EXIT_FAILURE;
	HX_zvecfree(argv);
	return EXIT_SUCCESS;
}
