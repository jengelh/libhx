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

static void t_mc(void)
{
	hxmc_t *s, *old_s;

	s = HXmc_meminit(NULL, 4096);
	printf("%" HX_SIZET_FMT "u\n", HXmc_length(s));
	if (HXmc_length(s) != 0)
		abort();
	old_s = s;
	HXmc_trunc(&s, 8192);
	if (old_s != s)
		fprintf(stderr, "INFO: HXmc: no reallocation took place.\n");
	printf("Length is now %" HX_SIZET_FMT "u\n", HXmc_length(s));
	HXmc_setlen(&s, 16384);
	printf("Length is now %" HX_SIZET_FMT "u\n", HXmc_length(s));
	HXmc_free(s);
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

static void t_strcpy(void)
{
	hxmc_t *vp = NULL;

	HXmc_strcpy(&vp, NULL);
	if (vp != NULL)
		abort();
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

/* avoid these being inlined */
extern char *f_strlcpy_str(char *, const char *, size_t);
extern char *f_strlcpy_mem(char *, const char *, size_t);

EXPORT_SYMBOL char *f_strlcpy_str(char *d, const char *s, size_t n)
{
	if (n == 0)
		return d;
	strncpy(d, s, n);
	d[n-1] = '\0';
	return d;
}

EXPORT_SYMBOL char *f_strlcpy_mem(char *dest, const char *src, size_t dsize)
{
	size_t slen = strlen(src);
	if (slen < dsize)
		return static_cast(char *, memcpy(dest, src, slen + 1));
	if (dsize > 0) {
		memcpy(dest, src, dsize - 1);
		dest[dsize-1] = '\0';
	}
	return dest;
}

static const char s_lorem_ipsum[] = /* 1368 chars */
"Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Aenean commodo "
"ligula eget dolor. Aenean massa. Cum sociis natoque penatibus et magnis dis "
"parturient montes, nascetur ridiculus mus. Donec quam felis, ultricies nec, "
"pellentesque eu, pretium quis, sem. Nulla consequat massa quis enim. Donec "
"pede justo, fringilla vel, aliquet nec, vulputate eget, arcu. In enim justo, "
"rhoncus ut, imperdiet a, venenatis vitae, justo. Nullam dictum felis eu pede "
"mollis pretium. Integer tincidunt. Cras dapibus. Vivamus elementum semper "
"nisi. Aenean vulputate eleifend tellus. Aenean leo ligula, porttitor eu, "
"consequat vitae, eleifend ac, enim. Aliquam lorem ante, dapibus in, viverra "
"quis, feugiat a, tellus. Phasellus viverra nulla ut metus varius laoreet. "
"Quisque rutrum. Aenean imperdiet. Etiam ultricies nisi vel augue. Curabitur "
"ullamcorper ultricies nisi. Nam eget dui. Etiam rhoncus. Maecenas tempus, "
"tellus eget condimentum rhoncus, sem quam semper libero, sit amet adipiscing "
"sem neque sed ipsum. Nam quam nunc, blandit vel, luctus pulvinar, hendrerit "
"id, lorem. Maecenas nec odio et ante tincidunt tempus. Donec vitae sapien ut "
"libero venenatis faucibus. Nullam quis ante. Etiam sit amet orci eget eros "
"faucibus tincidunt. Duis leo. Sed fringilla mauris sit amet nibh. Donec "
"sodales sagittis magna. Sed consequat, leo eget bibendum sodales, augue velit "
"cursus nunc,";

static void t_strlcpy(void)
{
	static const size_t picksizes[] =
		{4, 8, 16, 32, 64, 80, 128, 256, 1024, 2048};
	char ibuf[2048], obuf[2048];
	size_t ipick, opick, k, runs = 10000000 + HX_irand(0, 1);
	struct timespec start, stop, d1, d2, d3;

	for (ipick = 0; ipick < ARRAY_SIZE(picksizes); ++ipick) {
		/* Select string size */
		HX_strlcpy(ibuf, s_lorem_ipsum, picksizes[ipick]);

		for (opick = 0; opick < ARRAY_SIZE(picksizes); ++opick) {
			/* Select buffer size */
			clock_gettime(CLOCK_MONOTONIC, &start);
			for (k = 0; k < runs; ++k)
				f_strlcpy_str(reinterpret_cast(char *, obuf),
					ibuf, picksizes[opick]);
			clock_gettime(CLOCK_MONOTONIC, &stop);
			HX_timespec_sub(&d1, &stop, &start);

			clock_gettime(CLOCK_MONOTONIC, &start);
			for (k = 0; k < runs; ++k)
				f_strlcpy_mem(reinterpret_cast(char *, obuf),
					ibuf, picksizes[opick]);
			clock_gettime(CLOCK_MONOTONIC, &stop);
			HX_timespec_sub(&d2, &stop, &start);

			HX_timespec_sub(&d3, &d1, &d2);
			printf("%4zu->%4zu: " HX_TIMESPEC_FMT
			       " (str=" HX_TIMESPEC_FMT
			       " mem=" HX_TIMESPEC_FMT ")\n",
				strlen(ibuf), picksizes[opick],
				HX_TIMESPEC_EXP(&d3),
				HX_TIMESPEC_EXP(&d1),
				HX_TIMESPEC_EXP(&d2)
				);
		}
	}
}

static void t_strlcpy2(void)
{
	char a[3] = {49, 49, 49};
	HX_strlcpy(&a[1], &a[1], 0);
	assert(a[0] == 49 && a[0] == a[1] && a[1] == a[2]);
}

static void t_units(void)
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
			abort();
		}
		HX_unit_size(buf, ARRAY_SIZE(buf), vt[i].num, 1000, 9120);
		printf("\t%llu -> %s\n", vt[i].num, buf);
		if (strcmp(buf, vt[i].exp_1000) != 0) {
			printf("\texpected %s\n", vt[i].exp_1000);
			abort();
		}
	}
}

static void t_units_cu(void)
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
			abort();
		}
		HX_unit_size_cu(buf, ARRAY_SIZE(buf), vt[i].num, 1000);
		printf("\t%llu -> %s\n", vt[i].num, buf);
		if (strcmp(buf, vt[i].exp_1000) != 0) {
			printf("\texpected %s\n", vt[i].exp_1000);
			abort();
		}
	}
}

static void t_units_strto(void)
{
	static const struct {
		const char input[24];
		unsigned int exponent;
		unsigned long long expect_out;
		const char expect_rem[8];
	} vt[] = {
		{"-5k", 1000, ULLONG_MAX, "-5k"},
		{" -5.2k", 1000, ULLONG_MAX, "-5.2k"},
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
		{"0", 0, 0, ""},
		{"0k", 0, 0, ""},
		{"0  Z", 0, 0, ""},
		{"0.1k", 1000, 100, ""},
		{"0.1k", 1024, 102, ""},
		{" 0.1k", 1024, 102, ""},
		{"0.00000000000000001E", 1024, 11, ""},
		{"1.525444GiB", 1000, 1525444000, "iB"},
		{"1.525444GiB", 1024, 1637933022, "iB"},
	};
	char *end;
	for (size_t i = 0; i < ARRAY_SIZE(vt); ++i) {
		unsigned long long q = HX_strtoull_unit(vt[i].input, &end, vt[i].exponent);
		printf("%s -> %llu __ %s\n", vt[i].input, q, end);
		if (q != vt[i].expect_out || strcmp(end, vt[i].expect_rem) != 0)
			printf("BUG\n");
	}
}

static void t_time_units(void)
{
	static const struct {
		unsigned long long input;
		unsigned int flags;
		const char expect_out[24];
	} vt[] = {
		{31536000, 0, "365d"},
		{31622400, 0, "366d"},
		{31622400, HXUNIT_YEARS, "1y18h"},
		{31622400, HXUNIT_MONTHS, "1y"},
		{31622400, HXUNIT_WEEKS, "1y"},
		{31622400, HXUNIT_MONTHS | HXUNIT_WEEKS, "1y"},
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
		printf("%llus => \"%s\"\n", vt[i].input, ret);
		if (strcmp(ret, vt[i].expect_out) != 0)
			printf("\tBUG, expected \"%s\"\n", vt[i].expect_out);
	}
}

static void t_time_strto(void)
{
	#define NS_PER_S 1000000000ULL
	static const struct {
		const char *input;
		unsigned long long expect_s, expect_ns;
		const char expect_rem[8];
	} vt[] = {
		{"29µs", 0, 29000, ""},
		{"1y", 31557600, NS_PER_S * 31557600, ""},
		{"1y1month1week1d1h1min1s ", 31557600+2629800+86400*8+3600+60+1, NS_PER_S * (31557600+2629800+86400*8+3600+60+1), ""},
		{" -1d", 0, 0, "-1d"},
		{"1 -", 1, NS_PER_S, "-"},
		{"12.5 hours .5 hours 240 minutes 25200 seconds", 86400, NS_PER_S * 86400, ""},
		{"1s", 1, NS_PER_S, ""},
		{"1min", 60, 60 * NS_PER_S, ""},
		{"0", 0, 0, ""},
	};
	char *end;
	printf("===== t_time_strto\n");
	for (size_t i = 0; i < ARRAY_SIZE(vt); ++i) {
		unsigned long long q = HX_strtoull_sec(vt[i].input, &end);
		unsigned long long qn = HX_strtoull_nsec(vt[i].input, &end);
		printf("\"%s\" => %llus [%lluns] + \"%s\"\n", vt[i].input, q, qn, end);
		if (q != vt[i].expect_s || qn != vt[i].expect_ns)
			printf("\tBUG: expected %llus [%lluns]\n", vt[i].expect_s, vt[i].expect_ns);
		if (strcmp(end, vt[i].expect_rem) != 0)
			printf("\tBUG: expected remainder \"%s\"\n", vt[i].expect_rem);
	}
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

int main(int argc, const char **argv)
{
	hxmc_t *tx = NULL;
	const char *file = (argc >= 2) ? argv[1] : "tx-string.cpp";
	FILE *fp;

	if (HX_init() <= 0)
		abort();
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

	t_mc();
	t_path();
	t_strcpy();
	t_strncat();
	t_strnlen();
	t_strdup();
	t_strsep();
	t_strtrim();
	t_split();
	t_split2();
	t_units();
	t_units_cu();
	t_units_strto();
	t_time_units();
	t_time_strto();
	t_strlcpy();
	t_strlcpy2();
	HXmc_free(tx);
	HX_exit();
	return EXIT_SUCCESS;
}
