/* long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing */
// SPDX-License-Identifier: MIT
#ifndef __cplusplus
#	include <assert.h>
#	include <errno.h>
#	include <stddef.h>
#	include <stdio.h>
#	include <stdlib.h>
#	include <time.h>
#else
#	include <cassert>
#	include <cerrno>
#	include <cstddef>
#	include <cstdio>
#	include <cstdlib>
#	include <ctime>
#endif
#include <libHX/defs.h>
#include <libHX/init.h>
#include <libHX/misc.h>
#include <libHX/string.h>
#include "internal.h"

static int t_mc(void)
{
	hxmc_t *s, *old_s;

	s = HXmc_meminit(NULL, 4096);
	printf("%" HX_SIZET_FMT "u\n", HXmc_length(s));
	if (HXmc_length(s) != 0)
		return EXIT_FAILURE;
	old_s = s;
	HXmc_trunc(&s, 8192);
	if (old_s != s)
		fprintf(stderr, "INFO: HXmc: no reallocation took place.\n");
	printf("Length is now %" HX_SIZET_FMT "u\n", HXmc_length(s));
	HXmc_setlen(&s, 16384);
	printf("Length is now %" HX_SIZET_FMT "u\n", HXmc_length(s));
	HXmc_free(s);
	return EXIT_SUCCESS;
}

static void t_path(void)
{
	static const char *const paths[] = {
		".", "..", "/", "//", "/.", "/./", "/.//", "/./.",
		"/mnt", "//mnt", "/mnt/", "//mnt/", "//mnt//", "//mnt//root",
		"/mnt/.", "/mnt/./", "mnt", "mnt/", "mnt/root", "mnt/root/",
		"mnt//root", NULL
	};
	const char *const *iter;

	printf("#\tname\tbn\tbne\tdn\n");
	for (iter = paths; *iter != NULL; ++iter) {
		char *bn = HX_basename(*iter);
		char *bne = HX_basename_exact(*iter);
		char *dn = HX_dirname(*iter);
		printf("\t%s\t%s\t%s\t%s\n", *iter, bn, bne, dn);
		free(bne);
		free(dn);
	}
}

static int t_strcpy(void)
{
	hxmc_t *vp = NULL;

	HXmc_strcpy(&vp, NULL);
	return vp == nullptr ? EXIT_SUCCESS : EXIT_FAILURE;
}

static void t_strdup(void)
{
	char *a;
	a = HX_strndup("DATA", 2);
	printf(">%s<\n", a);
	free(a);
	a = HX_strndup("DATA", 10);
	printf(">%s<\n", a);
	free(a);
}

static void t_strncat(void)
{
	char data[5] = "DATA";

#ifndef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
#endif
	if (snprintf(data, sizeof(data), "12345678") >=
	    static_cast(ssize_t, sizeof(data)))
		printf("Not enough space\n");
#ifndef __clang__
#pragma GCC diagnostic pop
#endif
	printf("String: >%s<\n", data);

	HX_strlcat(data, "pqrstuv__", 2);
	printf("String: >%s<\n", data);

	*data = '\0';
	HX_strlcat(data, "123456789", sizeof(data));
	printf("String: >%s<\n", data);

	*data = '\0';
	HX_strlncat(data, "123456789", sizeof(data), 9);
	printf("String: >%s<\n", data);
}

static void t_strnlen(void)
{
	static const char s[] = "Hello world";
	printf("# strnlen: %" HX_SIZET_FMT "u %" HX_SIZET_FMT "u "
	       "%" HX_SIZET_FMT "u %" HX_SIZET_FMT "u %" HX_SIZET_FMT "u\n",
		HX_strnlen(s, -1), HX_strnlen(s, 0), HX_strnlen(s, 1),
		HX_strnlen(s, strlen(s)), HX_strnlen(s, 999));
}

static void t_strsep(void)
{
	char b[] = "jengelh:x:1500:100:Jan Engelhardt:/home/jengelh:/bin/bash";
	char *wp = b, *ret;

	printf("# strsep\n");
	while ((ret = HX_strsep2(&wp, ":")) != NULL)
		printf("%s\n", ret);
}

static void t_strtrim(void)
{
	char a[] = "  a and b  ", aexp[] = "a and b  ";
	char b[] = "  a and b  ", bexp[] = "  a and b";
	char c[] = "a&b", cexp[] = "a&b";
	const char *r;

	r = HX_stpltrim(a);
	printf("HX_stpltrim(\"%s\") = \"%s\"\n", a, r);
	assert(strcmp(r, aexp) == 0);

	printf("HX_strltrim(\"%s\") = ", a);
	printf("\"%s\"\n", (HX_strltrim(a), a));
	assert(strcmp(a, aexp) == 0);

	printf("HX_strrtrim(\"%s\") = ", b);
	printf("\"%s\"\n", (HX_strrtrim(b), b));
	assert(strcmp(b, bexp) == 0);

	assert(strcmp(cexp, HX_stpltrim(c)) == 0);
	assert(strcmp(cexp, (HX_strltrim(c), c)) == 0);
	assert(strcmp(cexp, (HX_strrtrim(c), c)) == 0);
}

static void t_split(void)
{
	char t1[] = "root:x:0:0:root:/root:/bin/bash";
	char t2[sizeof(t1)];
	int f0, f1, f2;
	char **a0, **a1, *a2[10];
	char *const *wp;

	memcpy(t2, t1, sizeof(t1));
	a0 = HX_split(t1, ":", &f0, 0);
	a1 = HX_split_inplace(t1, ":", &f1, 0);
	f2 = HX_split_fixed(t2, ":", ARRAY_SIZE(a2), a2);

	/* complete allocation */
	printf("HX_split1: a0[%p]:", a0);
	for (wp = a0; *wp != NULL; ++wp)
		printf(" %s[%p]", *wp, *wp);
	printf("\n");

	/* array allocated */
	printf("HX_split_inplace: a1[%p]:", a1);
	for (wp = a1; *wp != NULL; ++wp)
		printf(" %s[%p]", *wp, *wp);
	printf("\n");

	/* nothing allocated */
	printf("HX_split5: a2[%p]:", a2);
	for (wp = a2; f2 > 0; --f2, ++wp)
		printf(" %s[%p]", *wp, *wp);
	printf("\n");

	HX_zvecfree(a0);
	free(a1);
}

static void t_split2(void)
{
	static const char tmp[] = "";
	int c = 0;
	char **a;

	a = HX_split(tmp, " ", &c, 6);
	printf("Got %d fields\n", c);
	HX_zvecfree(a);
}

static void t_strlcpy2(void)
{
	char a[3] = {49, 49, 49};
	HX_strlcpy(&a[1], &a[1], 0);
	assert(a[0] == 49 && a[0] == a[1] && a[1] == a[2]);
}

static int t_units(void)
{
	static const struct {
		unsigned long long num;
		const char exp_1024[6], exp_1000[6];
	} vt[] = {
		{1023, "1023", "1023"},
		{1024, "1024", "1024"},
		{1945, "1945", "1945"},
		{1946, "1946", "1946"},
		{1022975, "998k", "1022k"},
		{1022976, "999k", "1022k"},
		{1022977, "999k", "1022k"},
		{1047552, "1023k", "1047k"},
		{1047553, "1023k", "1047k"},
		{1992294, "1945k", "1992k"},
		{1992295, "1945k", "1992k"},
		{1072693248, "1023M", "1072M"},
		{1072693249, "1023M", "1072M"},
		{ULLONG_MAX, "15E", "18E"},
	};
	char buf[HXSIZEOF_Z64+3];
	printf("unit_size:\n");

	for (size_t i = 0; i < ARRAY_SIZE(vt); ++i) {
		HX_unit_size(buf, ARRAY_SIZE(buf), vt[i].num, 1024, 9120);
		printf("\t%llu -> %s\n", vt[i].num, buf);
		if (strcmp(buf, vt[i].exp_1024) != 0) {
			printf("\texpected %s\n", vt[i].exp_1024);
			return EXIT_FAILURE;
		}
		HX_unit_size(buf, ARRAY_SIZE(buf), vt[i].num, 1000, 9120);
		printf("\t%llu -> %s\n", vt[i].num, buf);
		if (strcmp(buf, vt[i].exp_1000) != 0) {
			printf("\texpected %s\n", vt[i].exp_1000);
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}

static int t_units_cu(void)
{
	static const struct {
		unsigned long long num;
		const char exp_1024[6], exp_1000[6];
	} vt[] = {
		{1023, "1023", "1.1k"},
		{1024, "1.0k", "1.1k"},
		{1945, "1.9k", "2.0k"},
		{1946, "2.0k", "2.0k"},
		{1022975, "999k", "1.1M"},
		{1022976, "999k", "1.1M"},
		{1022977, "1000k", "1.1M"},
		{1047552, "1023k", "1.1M"},
		{1047553, "1.0M", "1.1M"},
		{1992294, "1.9M", "2.0M"},
		{1992295, "2.0M", "2.0M"},
		{1072693248, "1023M", "1.1G"},
		{1072693249, "1.0G", "1.1G"},
		{ULLONG_MAX, "16E", "19E"},
	};
	char buf[80];
	printf("unit_size_cu:\n");

	for (size_t i = 0; i < ARRAY_SIZE(vt); ++i) {
		HX_unit_size_cu(buf, ARRAY_SIZE(buf), vt[i].num, 1024);
		printf("\t%llu -> %s\n", vt[i].num, buf);
		if (strcmp(buf, vt[i].exp_1024) != 0) {
			printf("\texpected %s\n", vt[i].exp_1024);
			return EXIT_FAILURE;
		}
		HX_unit_size_cu(buf, ARRAY_SIZE(buf), vt[i].num, 1000);
		printf("\t%llu -> %s\n", vt[i].num, buf);
		if (strcmp(buf, vt[i].exp_1000) != 0) {
			printf("\texpected %s\n", vt[i].exp_1000);
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}

static int t_units_strto(void)
{
	static const struct {
		const char input[24];
		unsigned int exponent;
		unsigned long long expect_out;
		const char expect_rem[8];
	} vt[] = {
		{"-5k", 1000, -5000ULL, ""},
		{" -5.2k", 1000, -5200ULL, ""},
		{"1", 9999, 1, ""},
		{"1024", 9999, 1ULL << 10, ""},
		{"1048576", 9999, 1ULL << 20, ""},
		{"1073741824", 9999, 1ULL << 30, ""},
		{"1099511627776", 9999, 1ULL << 40, ""},
		{"1125899906842624", 9999, 1ULL << 50, ""},
		{"1152921504606846976", 9999, 1ULL << 60, ""},
		{"18446744073709551615", 9, ULLONG_MAX, ""},
		{"1k", 1000, 1000ULL, ""},
		{"1M", 1000, 1000000ULL, ""},
		{"1G", 1000, 1000000000ULL, ""},
		{"1T", 1000, 1000000000000ULL, ""},
		{"1P", 1000, 1000000000000000ULL, ""},
		{"1E", 1000, 1000000000000000000ULL, ""},
		{"1k", 1024, 1ULL << 10, ""},
		{"1M", 1024, 1ULL << 20, ""},
		{"1G", 1024, 1ULL << 30, ""},
		{"1T", 1024, 1ULL << 40, ""},
		{"1P", 1024, 1ULL << 50, ""},
		{"1E", 1024, 1ULL << 60, ""},
		{"15E", 1024, 15ULL << 60, ""},
		{"16E", 1024, ULLONG_MAX, ""},
		{"16.0E", 1024, ULLONG_MAX, ""},
		{"1Z", 1024, ULLONG_MAX, ""},
		{"0", 0, 0, ""},
		{"0k", 0, 0, ""},
		{"0  Z", 0, 0, ""},
		{"0.1k", 1000, 100, ""},
		{"0.1k", 1024, 102, ""},
		{" 0.1k", 1024, 102, ""},
		{"0.00000000000000001E", 1024, 11, ""},
		{"1.525444GiB", 1000, 1525444000, "iB"},
		{"1.525444GiB", 1024, 1637933022, "iB"},
		{"2M4k", 1000, 2000000, "4k"},
		{"18446744073709551614", 0, 18446744073709551614ULL, ""},
		{"18446744073709551615", 0, ULLONG_MAX, ""},
		{"18446744073709551616", 0, ULLONG_MAX, ""},
		{"-18446744073709551614", 0, 2, ""},
		{"-18446744073709551615", 0, 1, ""},
		{"-18446744073709551616", 0, ULLONG_MAX, ""},
	};
	char *end;
	for (size_t i = 0; i < ARRAY_SIZE(vt); ++i) {
		unsigned long long q = HX_strtoull_unit(vt[i].input, &end, vt[i].exponent);
		printf("Observed: %s -> %llu __ %s\n", vt[i].input, q, end);
		if (q != vt[i].expect_out || strcmp(end, vt[i].expect_rem) != 0) {
			printf("Expected: %s -> %llu __ %s\n", vt[i].input,
				vt[i].expect_out, vt[i].expect_rem);
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}

static int t_time_units(void)
{
	static const struct {
		unsigned long long input;
		unsigned int flags;
		const char expect_out[41];
	} vt[] = {
#define SECONDS_PER_YEAR 31557600 /* 365.25 days */
#define SECONDS_PER_MONTH 2629800 /* 1/12th of that year = 30.4375 days */
		{18446744073709551615ULL, 0, "213503982334601d7h15s"},
		{18446744073709526399ULL, 0, "213503982334600d23h59min59s"},
		{18446744073709180799ULL, HXUNIT_WEEKS, "30500568904942weeks2d23h59min59s"},
		{18446744073708451799ULL, HXUNIT_MONTHS, "7014504553087months2d23h59min59s"},
		{18446744073707636399ULL, HXUNIT_MONTHS | HXUNIT_WEEKS, "7014504553086months3weeks2d23h59min59s"},
		{18446744073688622999ULL, HXUNIT_YEARS | HXUNIT_MONTHS | HXUNIT_WEEKS, "584542046089y11months2weeks2d23h59min59s"},
		{31536000, 0, "365d"},
		{31622400, 0, "366d"},
		{34819200, HXUNIT_WEEKS, "57weeks4d"},
		{34819200, HXUNIT_MONTHS, "13months7d7h30min"},
		{34819200, HXUNIT_MONTHS | HXUNIT_WEEKS, "13months1week7h30min"},
		{34819200, HXUNIT_YEARS, "1y37d18h"},
		{34819200, HXUNIT_YEARS | HXUNIT_WEEKS, "1y5weeks2d18h"},
		{34819200, HXUNIT_YEARS | HXUNIT_MONTHS, "1y1month7d7h30min"},
		{34819200, HXUNIT_YEARS | HXUNIT_MONTHS | HXUNIT_WEEKS, "1y1month1week7h30min"},
		{2678400, HXUNIT_MONTHS, "1month13h30min"},
		{2592000, HXUNIT_MONTHS, "30d"},
		{608400, HXUNIT_WEEKS, "1week1h"},
		{90061, 0, "1d1h1min1s"},
		{3692, 0, "1h1min32s"},
		{67, 0, "1min7s"},
		{1, 0, "1s"},
		{0, 0, "0s"},
	};
	printf("===== HX_unit_seconds\n");
	for (size_t i = 0; i < ARRAY_SIZE(vt); ++i) {
		char out[60];
		char *ret = HX_unit_seconds(out, ARRAY_SIZE(out), vt[i].input, vt[i].flags);
		printf("Observed: %llus => \"%s\"\n", vt[i].input, ret);
		if (strcmp(ret, vt[i].expect_out) != 0) {
			printf("Expected: \"%s\"\n", vt[i].expect_out);
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}

static int t_time_strto(void)
{
	#define NS_PER_S 1000000000ULL
	static const struct {
		const char *input;
		unsigned long long expect_s, expect_ns;
		const char expect_rem[16];
	} vt[] = {
		{"29µs", 0, 29000, ""},
		{"1y", 31557600, NS_PER_S * 31557600, ""},
		{"1y1month1week1d1h1min1s ", 31557600+2629800+86400*8+3600+60+1, NS_PER_S * (31557600+2629800+86400*8+3600+60+1), ""},
		{" -1d", 0, 0, "-1d"},
		{"1 -", 0, 0, "1 -"},
		{"12.5 hours .5 hours 240 minutes 25200 seconds", 86400, NS_PER_S * 86400, ""},
		{"1s", 1, NS_PER_S, ""},
		{"1min", 60, 60 * NS_PER_S, ""},
		{"0", 0, 0, ""},
		{"0.0", 0, 0, ""},
		{"1s0", 1, NS_PER_S, ""},
		{"1s0.0", 1, NS_PER_S, ""},
		{"1s1s", 2, 2 * NS_PER_S, ""},
		{"1s1", 1, 1 * NS_PER_S, "1"},
		{"584542046091y", ULLONG_MAX, ULLONG_MAX, "584542046091y"},
	};
	char *end;
	printf("===== t_time_strto\n");
	for (size_t i = 0; i < ARRAY_SIZE(vt); ++i) {
		unsigned long long q = HX_strtoull_sec(vt[i].input, &end);
		unsigned long long qn = HX_strtoull_nsec(vt[i].input, &end);
		printf("Observed: \"%s\" => %llus [%lluns] + \"%s\"\n", vt[i].input, q, qn, end);
		if (q != vt[i].expect_s || qn != vt[i].expect_ns) {
			printf("Expected: %llus [%lluns]\n", vt[i].expect_s, vt[i].expect_ns);
			return EXIT_FAILURE;
		}
		if (strcmp(end, vt[i].expect_rem) != 0) {
			printf("Expected: remainder \"%s\"\n", vt[i].expect_rem);
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}

static int t_strmid(void)
{
#define T(spar,opar,lpar,xpar) do { \
		char *s = HX_strmid((spar), (opar), (lpar)); \
		if (s == nullptr) \
			return EXIT_FAILURE; \
		int ret = strcmp(s, (xpar)); \
		if (ret != 0) { \
			fprintf(stderr, "Faillure: substr %s,%d,%d = %s\n", \
				(spar), static_cast(int, (opar)), static_cast(int, (lpar)), s); \
			free(s); \
			return ret; \
		} \
		free(s); \
	} while (false)

	T("Hello World", -12, 5, "");
	T("bark", -3, -1, "ar");
	T("cake", -3, -3, "");
	T("cake", -3, -4, "");
	T("fun", 0, 0, "");
	T("bark", 0, 1, "b");
	T("bark", 0, 5, "bark");
	T("bark", -4, 1, "b");
	T("bark", -4, 5, "bark");
	return EXIT_SUCCESS;
#undef T
}

static int runner(int argc, char **argv)
{
	hxmc_t *tx = NULL;
	const char *file = (argc >= 2) ? argv[1] : "tx-string.cpp";
	FILE *fp;

	if (HX_init() <= 0)
		return EXIT_FAILURE;
	int ret = t_strmid();
	if (ret != EXIT_SUCCESS)
		return EXIT_FAILURE;

	fp = fopen(file, "r");
	if (fp == NULL) {
		fprintf(stderr, "Cannot open %s: %s\n", file, strerror(errno));
	} else {
		while (HX_getl(&tx, fp) != NULL)
			printf("%s", tx);
		fclose(fp);
	}

	ret = t_mc();
	if (ret != EXIT_SUCCESS)
		return EXIT_FAILURE;
	t_path();
	ret = t_strcpy();
	if (ret != EXIT_SUCCESS)
		return EXIT_FAILURE;
	t_strncat();
	t_strnlen();
	t_strdup();
	t_strsep();
	t_strtrim();
	t_split();
	t_split2();
	ret = t_units();
	if (ret != EXIT_SUCCESS)
		return EXIT_FAILURE;
	ret = t_units_cu();
	if (ret != EXIT_SUCCESS)
		return EXIT_FAILURE;
	ret = t_units_strto();
	if (ret != EXIT_SUCCESS)
		return EXIT_FAILURE;
	ret = t_time_units();
	if (ret != EXIT_SUCCESS)
		return EXIT_FAILURE;
	ret = t_time_strto();
	if (ret != EXIT_SUCCESS)
		return EXIT_FAILURE;
	t_strlcpy2();
	HXmc_free(tx);
	HX_exit();
	return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
	int ret = runner(argc, argv);
	if (ret == EXIT_FAILURE)
		fprintf(stderr, "FAILED\n");
	return ret;
}
