/* long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing */
/*
 *	Copyright Jan Engelhardt
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the WTF Public License version 2 or
 *	(at your option) any later version.
 */
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

	if (snprintf(data, sizeof(data), "12345678") >=
	    static_cast(ssize_t, sizeof(data)))
		printf("Not enough space\n");
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
			printf("%4zu->%4zu: %1ld.%09ld (str=%ld.%09ld mem=%ld.%09ld)\n",
				strlen(ibuf), picksizes[opick],
				static_cast(long, d3.tv_sec), d3.tv_nsec,
				static_cast(long, d1.tv_sec), d1.tv_nsec,
				static_cast(long, d2.tv_sec), d2.tv_nsec
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

int main(int argc, const char **argv)
{
	hxmc_t *tx = NULL;
	const char *file = (argc >= 2) ? argv[1] : "tx-string.cpp";
	FILE *fp;

	if (HX_init() <= 0)
		abort();

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
	t_strlcpy();
	t_strlcpy2();
	HXmc_free(tx);
	HX_exit();
	return EXIT_SUCCESS;
}
