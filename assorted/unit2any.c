#include <stdbool.h>
#include <stdlib.h>
#include <libHX/option.h>

static double dpi = 96;

static void px2any(const struct HXoptcb *cbi)
{
	double px = cbi->data_dbl;

	printf("%f px are (at %f DPI) equal to:\n", px, dpi);
	printf("\t%f inch\n", px / dpi);
	printf("\t%f pt\n", px * 72 / dpi);
	printf("\t%f cm\n", px * 2.54 / dpi);
}

static void pt2any(const struct HXoptcb *cbi)
{
	double pt = cbi->data_dbl;

	printf("%f pt are equal to:\n", pt);
	printf("\t%f inch\n", pt / 72);
	printf("\t%f px (at %f DPI)\n", dpi * pt / 72, dpi);
	printf("\t%f cm\n", pt * 2.54 / 72);
}

static const struct HXoption option_table[] = {
	{.sh = 'D', .ln = "dpi", .type = HXTYPE_DOUBLE, .ptr = &dpi,
	 .help = "Resolution (default: 96 dpi)"},
	{.sh = 'P', .ln = "px", .type = HXTYPE_DOUBLE, .cb = px2any},
	{.sh = 'p', .ln = "pt", .type = HXTYPE_DOUBLE, .cb = pt2any},
	HXOPT_AUTOHELP,
	HXOPT_TABLEEND,
};

static bool get_options(int *argc, const char ***argv)
{
}

int main(int argc, const char **argv)
{
	int ret;

	ret = HX_getopt(option_table, &argc, &argv, HXOPT_USAGEONERR);
	if (ret != HXOPT_ERR_SUCCESS)
		return EXIT_FAILURE;

	
}
