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
#include "internal.h"

static const struct HXoption rp_option_table[] = {
	{.sh = 'a', .type = HXTYPE_NONE, .help = "Produce an absolute path"},
	{.sh = 'p', .type = HXTYPE_NONE, .help = "Deactivate resolution of \"..\" entries"},
	{.sh = 's', .type = HXTYPE_NONE, .help = "Deactivate resolution of \".\" entries"},
	HXOPT_AUTOHELP,
	HXOPT_TABLEEND,
};

static void t_1(void)
{
	hxmc_t *tmp = HXmc_strinit("");
	/* two components, so that HX_readlink gets called twice */
	HX_realpath(&tmp, "/dev/tty", HX_REALPATH_DEFAULT);
	HXmc_free(tmp);
}

static void t_2(void)
{
	hxmc_t *tmp = HXmc_strinit("");
	int ret = HX_realpath(&tmp, "../../../../dev/tty", HX_REALPATH_ABSOLUTE | HX_REALPATH_DEFAULT);
	if (ret > 0)
		printf("t_2: %s\n", tmp);
	HXmc_free(tmp);
}

int main(int argc, char **argv)
{
	hxmc_t *res;
	int ret;
	struct HXopt6_result result;
	if (HX_getopt6(rp_option_table, argc, argv, &result,
	    HXOPT_USAGEONERR | HXOPT_ITER_OA) != HXOPT_ERR_SUCCESS)
		return false;
	unsigned int rp_flags = HX_REALPATH_DEFAULT;
	for (int i = 0; i < result.nopts; ++i) {
		switch (result.desc[i]->sh) {
		case 'a': rp_flags |= HX_REALPATH_ABSOLUTE; break;
		case 'p': rp_flags &= ~HX_REALPATH_PARENT; break;
		case 's': rp_flags &= ~HX_REALPATH_SELF; break;
		}
	}
	t_1();
	t_2();

	res = NULL;
	for (int i = 0; i < result.nargs; ++i) {
		ret = HX_realpath(&res, result.uarg[i], rp_flags);
		if (ret < 0) {
			perror("HX_realpath");
			printf("\n");
		} else {
			printf("%s\n", res);
		}
	}
	HX_getopt6_clean(&result);
	return EXIT_SUCCESS;
}
