/* long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing long line for testing */
/*
 *	this program is released in the Public Domain
 */
#ifndef __cplusplus
#	include <errno.h>
#	include <stddef.h>
#	include <stdio.h>
#	include <stdlib.h>
#else
#	include <cerrno>
#	include <cstddef>
#	include <cstdio>
#	include <cstdlib>
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
	printf("%zu\n", HXmc_length(s));
	if (HXmc_length(s) != 0)
		abort();
	old_s = s;
	HXmc_trunc(&s, 8192);
	if (old_s != s)
		fprintf(stderr, "INFO: HXmc: no reallocation took place.\n");
	printf("Length is now %zu\n", HXmc_length(s));
	HXmc_setlen(&s, 16384);
	printf("Length is now %zu\n", HXmc_length(s));
	HXmc_free(s);
}

static void t_path(void)
{
	static const char *const paths[] = {
		".", "..", "/", "//", "/.", "/./", "/./.",
		"/mnt", "//mnt", "/mnt/", "//mnt/", "//mnt//", "//mnt//root",
		"/mnt/.", "/mnt/./", "mnt", "mnt/", "mnt/root", "mnt/root/",
		"mnt//root", NULL
	};
	const char *const *iter;
	char *item;

	printf("# dirname\n");
	for (iter = paths; *iter != NULL; ++iter) {
		item = HX_dirname(*iter);
		printf("\t%s -> %s\n", *iter, item);
		free(item);
	}

	printf("# basename_exact\n");
	for (iter = paths; *iter != NULL; ++iter) {
		item = HX_basename_exact(*iter);
		printf("\t%s -> %s\n", *iter, item);
		free(item);
	}

	printf("# basename_fast\n");
	for (iter = paths; *iter != NULL; ++iter)
		printf("\t%s -> %s\n", *iter, HX_basename(*iter));
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
	printf("# strnlen: %zu %zu %zu %zu %zu\n",
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

static void t_split(void)
{
	char t1[] = "root:x:0:0:root:/root:/bin/bash";
	char t2[sizeof(t1)];
	int f0, f1, f2;
	char **a0, **a1, *a2[10];
	char *const *wp;

	memcpy(t2, t1, sizeof(t1));
	a0 = HX_split(t1, ":", &f0, 0);
	a1 = HX_split4(t1, ":", &f1, 0);
	f2 = HX_split5(t2, ":", ARRAY_SIZE(a2), a2);

	/* complete allocation */
	printf("HX_split1: a0[%p]:", a0);
	for (wp = a0; *wp != NULL; ++wp)
		printf(" %s[%p]", *wp, *wp);
	printf("\n");

	/* array allocated */
	printf("HX_split4: a1[%p]:", a1);
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

static void t_quote(void)
{
	char *fm = NULL;
	printf("%p %p\n", "a", HX_strquote("a", 0xffff, &fm));
	printf("%s\n", HX_strquote("\"Good\" ol' \\'escaped\\' strings", HXQUOTE_SQUOTE, &fm));
	printf("%s\n", HX_strquote("\"Good\" ol' \\'escaped\\' strings", HXQUOTE_DQUOTE, &fm));
	printf("%s\n", HX_strquote("<p style=\"height: 1;\">Foo &amp; \"bar\"</p>", HXQUOTE_HTML, &fm));
	printf("(objectClass=%s)\n", HX_strquote(" #o=foo(*),bar ", HXQUOTE_LDAPFLT, &fm));
	printf("o=%s\n", HX_strquote(" #o=foo(*),bar ", HXQUOTE_LDAPRDN, &fm));
	printf("%s\n", HX_strquote("whatever", HXQUOTE_BASE64, &fm));
	free(fm);
	if (HX_strquote("", -1, NULL) != NULL)
		printf("Something is fishy with strquote\n");
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
	t_split();
	t_split2();
	t_quote();
	HXmc_free(tx);
	HX_exit();
	return EXIT_SUCCESS;
}
