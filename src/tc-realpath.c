// SPDX-License-Identifier: MIT
/*
 *	Test utility for libHX's realpath
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <libHX/io.h>
#include <libHX/option.h>
#include <libHX/string.h>

static unsigned int rp_flags;
static unsigned int rp_absolute;
static unsigned int rp_no_parent, rp_no_self;

static const struct HXoption rp_option_table[] = {
	{.sh = 'a', .type = HXTYPE_NONE, .ptr = &rp_absolute,
	 .help = "Produce an absolute path"},
	{.sh = 'p', .type = HXTYPE_NONE, .ptr = &rp_no_parent,
	 .help = "Deactivate resolution of \"..\" entries"},
	{.sh = 's', .type = HXTYPE_NONE, .ptr = &rp_no_self,
	 .help = "Deactivate resolution of \".\" entries"},
	HXOPT_AUTOHELP,
	HXOPT_TABLEEND,
};

static bool rp_get_options(int *argc, char ***argv)
{
	if (HX_getopt(rp_option_table, argc, argv, HXOPT_USAGEONERR) !=
	    HXOPT_ERR_SUCCESS)
		return false;
	rp_flags = HX_REALPATH_DEFAULT;
	if (rp_absolute)
		rp_flags |= HX_REALPATH_ABSOLUTE;
	if (rp_no_parent)
		rp_flags &= ~HX_REALPATH_PARENT;
	if (rp_no_self)
		rp_flags &= ~HX_REALPATH_SELF;
	return true;
}

static void t_1(void)
{
	hxmc_t *tmp = HXmc_strinit("");
	/* two components, so that HX_readlink gets called twice */
	HX_realpath(&tmp, "/dev/tty", HX_REALPATH_DEFAULT);
	HXmc_free(tmp);
}

int main(int argc, char **argv)
{
	hxmc_t *res;
	int ret;

	if (!rp_get_options(&argc, &argv))
		return EXIT_FAILURE;
	t_1();

	res = NULL;
	for (int i = 1; i < argc; ++i) {
		ret = HX_realpath(&res, argv[argc], rp_flags);
		if (ret < 0) {
			perror("HX_realpath");
			printf("\n");
		} else {
			printf("%s\n", res);
		}
	}
	HX_zvecfree(argv);
	return EXIT_SUCCESS;
}
