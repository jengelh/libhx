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

static int t_pthru(void)
{
	const char *argv[] = {
		"ARGV0", "-Zomg", "-GZfoo", "bar",
		"--unknown-f=13.37", "--unknown-a",
		"foo", "bar", NULL
	};
	char **nargv = nullptr;
	int nargc = 0;

	printf("PTHRU test:\n");
	if (HX_getopt5(table, const_cast(char **, argv), &nargc, &nargv,
	    HXOPT_USAGEONERR | HXOPT_PTHRU) != HXOPT_ERR_SUCCESS)
		return EXIT_FAILURE;
	printf("argc = %d\n", nargc);
	dump_argv(nargv);
	printf("\n");
	HX_zvecfree(nargv);
	return EXIT_SUCCESS;
}

static int t_empty_argv(void)
{
	char *zero_argv[] = {nullptr};
	char **new_argv = nullptr;

	printf("Testing argv={NULL}\n");
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
	char **nargv = nullptr;
	int ret = HX_getopt5(table, argv, &argc, &nargv, HXOPT_USAGEONERR);
	printf("Return value of HX_getopt: %d\n", ret);
	if (ret == EXIT_SUCCESS)
		HX_zvecfree(nargv);
	ret = t_empty_argv();
	if (ret != EXIT_SUCCESS)
		return ret;
	ret = t_keep_argv();
	if (ret != EXIT_SUCCESS)
		return ret;

	printf("Either-or is: %s\n", opt_eitheror[opt_dst]);
	printf("values: D=%lf I=%d L=%ld S=%s\n",
	       opt_kdbl, opt_kint, opt_klong, opt_kstr);
	printf("Verbosity level: %d\n", opt_v);
	printf("Mask: 0x%08X\n", opt_mask);
	printf("mcstr: >%s<\n", opt_mcstr);
	return t_pthru();
}

int main(int argc, char **argv)
{
	if (HX_init() <= 0)
		return EXIT_FAILURE;
	int ret = runner(argc, argv);
	if (ret != EXIT_SUCCESS)
		printf("FAILED\n");
	HX_exit();
	return ret;
}
