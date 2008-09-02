/*
 *	libHX/string.c
 *	Copyright © Jan Engelhardt <jengelh [at] gmx de>, 1999 - 2008
 *
 *	This file is part of libHX. libHX is free software; you can
 *	redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2 or 3 of the License.
 */
#include <sys/types.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX/string.h>
#include "internal.h"

static inline unsigned int min_uint(unsigned int a, unsigned int b)
{
	return (a < b) ? a : b;
}

EXPORT_SYMBOL char *HX_basename(const char *s)
{
	const char *p;
	if ((p = strrchr(s, '/')) != NULL)
		return const_cast(char *, p + 1);
	return const_cast(char *, s);
}

EXPORT_SYMBOL char *HX_chomp(char *s)
{
	char *p = s + strlen(s) - 1;
	while (p >= s) {
		if (*p != '\n' && *p != '\r')
			break;
		*p-- = '\0';
	}
	return s;
}

EXPORT_SYMBOL char *HX_dirname(const char *s)
{
	const char *last, *stop;
	char *p;

	if (*s == '\0')
		return HX_strdup(".");

	for (last = s + strlen(s) - 1; last > s && *last == '/'; --last)
		;

	if ((stop = HX_strbchr(s, last, '/')) == NULL)
		return HX_strdup(".");

	for (; stop > s && *stop == '/'; --stop)
		;

	p = HX_memdup(s, stop - s + 2);
	p[stop-s+1] = '\0';
	return p;
}

EXPORT_SYMBOL hmc_t *HX_getl(hmc_t **ptr, FILE *fp)
{
	/* Read a whole line, using a HMC string as dynamic buffer. */
	char temp[MAXLNLEN];

	if (fgets(temp, sizeof(temp), fp) == NULL)
		return NULL;

	if (*ptr == NULL)
		*ptr = hmc_sinit("");
	else
		hmc_trunc(ptr, 0);

	do {
		hmc_strcat(ptr, temp);
		if (strchr(temp, '\n') != NULL)
			break;
	} while (fgets(temp, sizeof(temp), fp) != NULL);

	return *ptr;
}

EXPORT_SYMBOL char **HX_split(const char *str, const char *delim,
    int *cp, int max)
{
	/*
	 * @countp can be NULL in case you are not interested in the number of
	 * fields. In either case, you can find out the number of fields by
	 * scanning through the resulting vector.
	 */
	int count = 0;
	char **ret;

	if (cp == NULL)
		cp = &count;
	*cp = 1;

	{
		const char *wp = str;
		while ((wp = strpbrk(wp, delim)) != NULL) {
			++*cp;
			++wp;
		}
	}

	if (max == 0)
		max = *cp;
	else if (*cp > max)
		*cp = max;

	ret = malloc(sizeof(char *) * (*cp + 1));
	ret[*cp] = NULL;

	{
		char *seg, *wp = HX_strdup(str), *bg = wp;
		size_t i = 0;

		while (--max > 0) {
			seg      = HX_strsep(&wp, delim);
			ret[i++] = HX_strdup(seg);
		}

		ret[i++] = HX_strdup(wp);
		free(bg);
	}

	return ret;
}

EXPORT_SYMBOL int HX_split5(char *s, const char *delim, int max, char **stk)
{
	/*
	 * HX_split5 - the "stack split" (we try to avoid using the heap):
	 * Split @s (modifies it, so must be writable!) at @delim with at most
	 * @max fields and putting the results into @stk[0..@max-1].
	 *
	 * Example on @max:
	 *	char *stk[max];
	 *	HX_split5(s, delim, max, stk);
	 */
	int i = 0;
	char *p;

	while (--max > 0) {
		if ((p = strpbrk(s, delim)) == NULL)
			break;
		stk[i++] = s;
		*p = '\0';
		s  = p + 1;
	}

	stk[i++] = s;
	return i;
}

EXPORT_SYMBOL char *HX_strbchr(const char *start, const char *now, char d)
{
	/* Find the last occurrence of @d within @start and @now. */
	while (now >= start)
		if (*now-- == d)
			return (char *)++now;
	return NULL;
}

EXPORT_SYMBOL char *HX_strclone(char **pa, const char *pb)
{
	if (*pa == pb)
		return *pa;
	if (*pa != NULL) {
		free(*pa);
		*pa = NULL;
	}
	if (pb == NULL)
		return NULL;
	if ((*pa = malloc(strlen(pb) + 1)) == NULL)
		return NULL;
	strcpy(*pa, pb);
	return *pa;
}

EXPORT_SYMBOL char *HX_strlower(char *expr)
{
	char *orig = expr;
	while (*expr != '\0') {
		*expr = tolower(*expr);
		++expr;
	}
	return orig;
}

EXPORT_SYMBOL size_t HX_strltrim(char *expr)
{
	char *travp;
	size_t diff = 0;
	travp = expr;

	while (*travp != '\0' && isspace(*travp))
		++travp;
	if ((diff = travp - expr) > 0)
		memmove(expr, travp, diff);
	return diff;
}

/* supports negative offsets like scripting languages */
EXPORT_SYMBOL char *HX_strmid(const char *expr, long offset, long length)
{
	char *buffer;

	if (offset < 0)
		offset = strlen(expr) + offset;
	if (length == 0)
		length = strlen(expr) - offset;
	else if (length < 0)
		length = strlen(expr) - offset + length;
	if ((buffer = malloc(length + 1)) == NULL)
		return NULL;

	expr += offset;
	return HX_strlcpy(buffer, expr, length + 1);
}

EXPORT_SYMBOL size_t HX_strrcspn(const char *s, const char *rej)
{
	size_t n = strlen(s);
	const char *p = s + n;
	while (--p >= s)
		if (strchr(rej, *p) != NULL)
			return p - s;
	return n;
}

EXPORT_SYMBOL char *HX_strrev(char *expr)
{
	char *dyn = HX_strdup(expr), *orig = expr;
	size_t s = strlen(dyn);

	dyn += s;
	while (s--)
		*expr++ = *--dyn;

	free(dyn);
	return orig;
}

EXPORT_SYMBOL size_t HX_strrtrim(char *expr)
{
	int i = strlen(expr), s = 0;
	while (i-- && isspace(expr[i]))
		++s;
	expr[++i] = '\0';
	return s;
}

EXPORT_SYMBOL char *HX_strsep(char **sp, const char *d)
{
	char *begin, *end;

	if (*sp == NULL || **sp == '\0')
		return NULL;
	begin = *sp;

	if (d[0] == '\0' || d[1] == '\0') {
		if (*begin == *d)
			end = begin;
		else if (*begin == '\0')
			end = NULL;
		else
			end = strchr(begin + 1, *d);
	} else {
		end = strpbrk(begin, d);
	}

	if (end == NULL) {
		*sp = NULL;
	} else {
		*end++ = '\0';
		*sp = end;
	}
	
	return begin;
}

EXPORT_SYMBOL char *HX_strsep2(char **wp, const char *str)
{
	char *ptr, *ret;
	if (*wp == NULL)
		return NULL;
	ret = *wp;
	if ((ptr = strstr(*wp, str)) == NULL) {
		*wp = NULL;
		return ret;
	}
	*ptr = '\0';
	*wp  = ptr + strlen(str);
	return ret;
}

EXPORT_SYMBOL char *HX_strupper(char *expr)
{
	char *orig = expr;
	while (*expr != '\0') {
		*expr = toupper(*expr);
		++expr;
	}
	return orig;
}
