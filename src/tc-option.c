// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2022 Jan Engelhardt
/*
 *	option parser test program
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX/defs.h>
#include <libHX/init.h>
#include <libHX/map.h>
#include <libHX/option.h>
#include "internal.h"

static int opt_v = 0, opt_mask = 0;
static char *opt_kstr = NULL;
static long opt_klong = 0;
static double opt_kdbl = 0;
static int opt_kflag = 0, opt_kint = 0;
static int opt_dst = 0;
static hxmc_t *opt_mcstr = NULL;

static void opt_cbf(const struct HXoptcb *cbi)
{
	printf("cbf was called... with \"%s\"/'%c'\n",
	       cbi->current->ln, cbi->current->sh);
}

static const char *opt_eitheror[] = {"neither", "either", "or"};
static struct HXoption table[] = {
	{.ln = "dbl", .type = HXTYPE_DOUBLE, .cb = opt_cbf,
	 .ptr = &opt_kdbl, .help = "Callback function for doubles"},
	{.ln = "flag", .sh = 'F', .type = HXTYPE_NONE, .cb = opt_cbf,
	 .ptr = &opt_kflag, .help = "Callback function for flags"},
	{.ln = "long", .sh = 'L', .type = HXTYPE_LONG, .cb = opt_cbf,
	 .ptr = &opt_klong, .help = "Callback function for integers"},
	{.sh = 'B', .type = HXTYPE_BOOL, .ptr = &opt_v,
	 .cb = opt_cbf, .help = "Bool test", .htyp = "value"},
	{.sh = 'P', .type = HXTYPE_MCSTR, .ptr = &opt_mcstr,
	 .help = "Any string"},
	{.ln = "str", .sh = 'S', .type = HXTYPE_STRING, .cb = opt_cbf,
	 .ptr = &opt_kstr, .help = "Callback function for strings"},
	{.ln = "either", .type = HXTYPE_VAL, .cb = opt_cbf, .ptr = &opt_dst,
	 .val = 1, .help = "Mutually exclusive selection: either | or"},
	{.ln = "or", .type = HXTYPE_VAL, .ptr = &opt_dst, .val = 2,
	 .cb = opt_cbf, .help = "Mutually exclusive selection: either | or"},
	{.ln = "quiet", .sh = 'q', .type = HXOPT_DEC, .ptr = &opt_v,
	 .cb = opt_cbf, .help = "Decrease verbosity"},
	{.ln = "quack", .type = HXOPT_INC, .ptr = &opt_v,
	 .cb = opt_cbf, .help = "Increase verbosity"},
	{.ln = "verbose", .sh = 'v', .type = HXOPT_INC, .ptr = &opt_v,
	 .cb = opt_cbf, .help = "Increase verbosity"},
	{.sh = 'A', .type = HXTYPE_INT | HXOPT_AND, .ptr = &opt_mask,
	 .cb = opt_cbf, .help = "AND mask test", .htyp = "value"},
	{.sh = 'O', .type = HXTYPE_INT | HXOPT_OR, .ptr = &opt_mask,
	 .cb = opt_cbf, .help = "OR mask test", .htyp = "value"},
	{.sh = 'X', .type = HXTYPE_INT | HXOPT_XOR, .ptr = &opt_mask,
	 .cb = opt_cbf, .help = "XOR mask test", .htyp = "value"},
	{.sh = 'G', .type = HXTYPE_NONE, .help = "Just a flag", .cb = opt_cbf},
	{.sh = 'H', .type = HXTYPE_NONE, .help = "Just a flag", .cb = opt_cbf},
	{.sh = 'I', .type = HXTYPE_NONE, .help = "Just a flag", .cb = opt_cbf},
	HXOPT_AUTOHELP,
	{.sh = 'J', .type = HXTYPE_NONE, .help = "Just a flag", .cb = opt_cbf},
	HXOPT_TABLEEND,
};

static void dump_argv(char **v)
{
	while (*v != NULL)
		printf("[%s] ", *v++);
	printf("\n");
}

static int t_empty_argv(void)
{
	char *zero_argv[] = {nullptr};
	char **new_argv = nullptr;

	printf("...with argv={NULL}\n");
	if (HX_getopt5(table, zero_argv, nullptr, &new_argv,
	    HXOPT_USAGEONERR) != HXOPT_ERR_SUCCESS)
		return EXIT_FAILURE;
	HX_zvecfree(new_argv);
	return EXIT_SUCCESS;
}

static int t_keep_argv(void)
{
	static const char *const one_argv[] = {"what", nullptr};
	const char **argv = const_cast2(const char **, one_argv);
	if (HX_getopt(table, nullptr, &argv, HXOPT_KEEP_ARGV) != HXOPT_ERR_SUCCESS)
		return EXIT_FAILURE;
	return argv == one_argv ? EXIT_SUCCESS : EXIT_FAILURE;
}

static int runner(int argc, char **argv)
{
	printf("== HX_getopt5 ==\n");
	char **nargv = nullptr;
	int ret = HX_getopt5(table, argv, &argc, &nargv, HXOPT_USAGEONERR);
	printf("Return value of HX_getopt: %d\n", ret);
	printf("Either-or is: %s\n", opt_eitheror[opt_dst]);
	printf("values: D=%lf I=%d L=%ld S=%s\n",
	       opt_kdbl, opt_kint, opt_klong, opt_kstr);
	printf("Verbosity level: %d\n", opt_v);
	printf("Mask: 0x%08X\n", opt_mask);
	printf("mcstr: >%s<\n", opt_mcstr);
	printf("new_argv:\n");
	for (char **p = nargv; p != nullptr && *p != nullptr; ++p)
		printf("\t%s\n", *p);
	if (ret == EXIT_SUCCESS)
		HX_zvecfree(nargv);

	printf("\n== getopt other tests ==\n");
	ret = t_empty_argv();
	if (ret != EXIT_SUCCESS)
		return ret;
	ret = t_keep_argv();
	if (ret != EXIT_SUCCESS)
		return ret;

	return EXIT_SUCCESS;
}

static int t_getopt6_aflags(int unused_argc, char **unused_argv)
{
	struct HXopt6_result result;
	char *argv[] = {"./prog", "-q", "foo", "-qS", "quux", "bar", "--NIL", nullptr};

	printf("== ANY_ORDER ==\n");
	int ret = HX_getopt6(table, 6, argv, &result, HXOPT_ANY_ORDER);
	if (ret != HXOPT_ERR_SUCCESS ||
	    result.uarg != nullptr || result.dup_argv != nullptr)
		return EXIT_FAILURE;
	HX_getopt6_clean(&result);

	/* T: Asking for DUP should produce data */
	/* T: Limit strings -- NIL must not be processed */
	printf("== ANY_ORDER/DUP_ARGS ==\n");
	ret = HX_getopt6(table, 6, argv, &result, HXOPT_ANY_ORDER | HXOPT_DUP_ARGS);
	if (ret != HXOPT_ERR_SUCCESS ||
	    result.uarg != nullptr ||
	    result.dup_argv == nullptr || result.dup_argc != 3)
		return EXIT_FAILURE;
	if (strcmp(result.dup_argv[0], argv[0]) != 0 ||
	    strcmp(result.dup_argv[1], argv[2]) != 0 ||
	    strcmp(result.dup_argv[2], argv[5]) != 0)
		return EXIT_FAILURE;
	for (int i = 0; i < result.dup_argc; ++i)
		printf(" %s", result.dup_argv[i]);
	printf("\n");
	HX_getopt6_clean(&result);

	/* T: POSIX order */
	printf("== RQ_ORDER/DUP_ARGS ==\n");
	ret = HX_getopt6(table, 6, argv, &result, HXOPT_RQ_ORDER | HXOPT_DUP_ARGS);
	if (ret != HXOPT_ERR_SUCCESS ||
	    result.uarg != nullptr ||
	    result.dup_argv == nullptr || result.dup_argc != 5)
		return EXIT_FAILURE;
	if (strcmp(result.dup_argv[0], argv[0]) != 0 ||
	    strcmp(result.dup_argv[1], argv[2]) != 0 ||
	    strcmp(result.dup_argv[2], argv[3]) != 0 ||
	    strcmp(result.dup_argv[3], argv[4]) != 0 ||
	    strcmp(result.dup_argv[4], argv[5]) != 0)
		return EXIT_FAILURE;
	for (int i = 0; i < result.dup_argc; ++i)
		printf(" %s", result.dup_argv[i]);
	printf("\n");
	HX_getopt6_clean(&result);

	/* T: Asking for ITER should produce data */
	printf("== ANY_ORDER/ITER_ARGS ==\n");
	ret = HX_getopt6(table, 6, argv, &result, HXOPT_ANY_ORDER | HXOPT_ITER_ARGS);
	if (ret != HXOPT_ERR_SUCCESS || result.dup_argv != nullptr ||
	    result.nargs != 2 || result.uarg == nullptr)
		return EXIT_FAILURE;
	if (strcmp(result.uarg[0], argv[2]) != 0 ||
	    strcmp(result.uarg[1], argv[5]) != 0)
		return EXIT_FAILURE;
	for (int i = 0; i < result.nargs; ++i)
		printf(" %s", result.uarg[i]);
	printf("\n");
	HX_getopt6_clean(&result);
	return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
	if (HX_init() <= 0)
		return EXIT_FAILURE;
	int ret = runner(argc, argv);
	if (ret != EXIT_SUCCESS)
		printf("FAILED\n");
	ret = t_getopt6_aflags(argc, argv);
	if (ret != EXIT_SUCCESS)
		printf("FAILED\n");
	HX_exit();
	return ret;
}
