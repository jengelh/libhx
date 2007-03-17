/*
	libHX/string.c
	Copyright © Jan Engelhardt <jengelh [at] gmx de>, 1999 - 2007

	This file is part of libHX. libHX is free software; you can
	redistribute it and/or modify it under the terms of the GNU
	Lesser General Public License as published by the Free Software
	Foundation; however ONLY version 2 of the License. For details,
	see the file named "LICENSE.LGPL2".
*/
#include <sys/types.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libHX.h"

/* Functions */
static inline unsigned int min_uint(unsigned int, unsigned int);
static char *replace_xt(char *, size_t, const struct HXoption *);

//-----------------------------------------------------------------------------
EXPORT_SYMBOL char *HX_basename(const char *s)
{
	char *p;
	if((p = strrchr(s, '/')) != NULL)
		return p + 1;
	return (char *)s;
}

EXPORT_SYMBOL char *HX_chomp(char *s)
{
	size_t len = strlen(s);
	char *p = s + len - 1;
	while(p >= s) {
		if(*p != '\n' && *p != '\r')
			break;
		*p-- = '\0';
	}
	return s;
}

EXPORT_SYMBOL char *HX_dirname(const char *s)
{
	char *dir;
	if(strrchr(s, '/') == NULL)
		return HX_strdup(".");

	dir = HX_strdup(s);
	*(strrchr(dir, '/')) = '\0';
	return dir;
}

EXPORT_SYMBOL hmc_t *HX_getl(hmc_t **ptr, FILE *fp)
{
	/* Read a whole line, using a HMC string as dynamic buffer. */
	char temp[MAXLNLEN];
	int ln = 0;

	if(*ptr == NULL)
		*ptr = hmc_sinit("");
	else
		hmc_trunc(ptr, 0);

	while(fgets(temp, sizeof(temp), fp) != NULL) {
		++ln;
		hmc_strcat(ptr, temp);
		if(strchr(temp, '\n') != NULL)
			break;
	}

	if(ln == 0)
		return NULL;
	return *ptr;
}

EXPORT_SYMBOL char **HX_split(const char *str, const char *delim,
  int *cp, int max)
{
	/* COUNTP can be NULL in case you are not interested in the number of
	fields. In either case, you can find out the number of fields by scanning
	through the resulting vector. */
	int count = 0;
	char **ret;

	if(cp == NULL) cp = &count;
	*cp = 1;

	{
		const char *wp = str;
		while((wp = strpbrk(wp, delim)) != NULL) {
			++*cp;
			++wp;
		}
	}

	if(max == 0)	   max = *cp;
	else if(*cp > max) *cp = max;
	ret = malloc(sizeof(char *) * (*cp + 1));
	ret[*cp] = NULL;

	{
		char *seg, *wp = HX_strdup(str), *bg = wp;
		size_t i = 0;

		while(--max > 0) {
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
	/* HX_split5 - the "stack split" (we try to avoid using the heap):
	Split S (modifies it, so must be writable!) at DELIM with at most MAX
	fields and putting the results into STK[0..MAX-1].
	Example on MAX:
		char *stk[max];
		HX_split5(s, delim, max, stk);
	*/
	int i = 0;
	char *p;

	while(--max > 0) {
		if((p = strpbrk(s, delim)) == NULL)
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
	/* Find the last occurrence of D within START and NOW. */
	while(now >= start)
		if(*now-- == d)
			return (char *)++now;
	return NULL;
}

EXPORT_SYMBOL char *HX_strclone(char **pa, const char *pb)
{
	if(*pa == pb)
		return *pa;
	if(*pa != NULL) {
		free(*pa);
		*pa = NULL;
	}
	if(pb == NULL)
		return NULL;
	if((*pa = malloc(strlen(pb) + 1)) == NULL)
		return NULL;
	strcpy(*pa, pb);
	return *pa;
}

EXPORT_SYMBOL char *HX_strlower(char *expr)
{
	char *orig = expr;
	while(*expr != '\0') {
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

	while(*travp != '\0' && isspace(*travp))
		++travp;
	if((diff = travp - expr) > 0)
		memmove(expr, travp, diff);
	return diff;
}

EXPORT_SYMBOL char *HX_strmid(const char *expr, long offset, long length)
{
	char *buffer;

	if(offset < 0)
		offset = strlen(expr) + offset;
	if(length == 0)
		length = strlen(expr) - offset;
	else if(length < 0)
		length = strlen(expr) - offset + length;
	if((buffer = malloc(length + 1)) == NULL)
		return NULL;

	expr += offset;
	return HX_strlcpy(buffer, expr, length + 1);
}

EXPORT_SYMBOL size_t HX_strrcspn(const char *s, const char *rej)
{
	size_t n = strlen(s);
	const char *p = s + n;
	while(--p >= s)
		if(strchr(rej, *p) != NULL)
			return p - s;
	return n;
}

EXPORT_SYMBOL char *HX_strrev(char *expr)
{
	char *dyn = HX_strdup(expr), *orig = expr;
	size_t s = strlen(dyn);

	dyn += s;
	while(s--)
		*expr++ = *--dyn;

	free(dyn);
	return orig;
}

EXPORT_SYMBOL size_t HX_strrtrim(char *expr)
{
	int i = strlen(expr), s = 0;
	while(i-- && isspace(expr[i]))
		++s;
	expr[++i] = '\0';
	return s;
}

EXPORT_SYMBOL char *HX_strsep(char **sp, const char *d)
{
	char *begin, *end;

	if(*sp == NULL || **sp == '\0')
		return NULL;
	begin = *sp;

	if(d[0] == '\0' || d[1] == '\0') {
		if(*begin == *d)
			end = begin;
		else if(*begin == '\0')
			end = NULL;
		else
			end = strchr(begin + 1, *d);
	} else {
		end = strpbrk(begin, d);
	}

	if(end == NULL) {
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
	if(*wp == NULL) return NULL;
	ret = *wp;
	if((ptr = strstr(*wp, str)) == NULL) {
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
	while(*expr != '\0') {
		*expr = toupper(*expr);
		++expr;
	}
	return orig;
}

//-----------------------------------------------------------------------------
static inline unsigned int min_uint(unsigned int a, unsigned int b)
{
	return (a < b) ? a : b;
}

static char *replace_xt(char *buf, size_t s, const struct HXoption *mt)
{
#define NTYPE(constant, fmt, type) \
	case (constant): \
		snprintf(buf, s, (fmt), *(type *)mt->ptr); \
		break;
	switch(mt->type) {
		case HXTYPE_BOOL:
			snprintf(buf, s, "%s", !!*(int *)mt->ptr ? "yes" : "no");
			break;
		NTYPE(HXTYPE_UCHAR,  "%u",   unsigned char);
		NTYPE(HXTYPE_CHAR,   "%d",   char);
		NTYPE(HXTYPE_USHORT, "%u",   unsigned short);
		NTYPE(HXTYPE_SHORT,  "%d",   short);
		NTYPE(HXTYPE_UINT,   "%u",   unsigned int);
		NTYPE(HXTYPE_INT,    "%d",   int);
		NTYPE(HXTYPE_ULONG,  "%lu",  unsigned long);
		NTYPE(HXTYPE_LONG,   "%ld",  long);
#ifndef _MSC_VER
		NTYPE(HXTYPE_ULLONG, "%llu", unsigned long long);
		NTYPE(HXTYPE_LLONG,  "%lld", long long);
#endif
		NTYPE(HXTYPE_FLOAT,  "%f",   float);
		NTYPE(HXTYPE_DOUBLE, "%f",   double);
		default:
			fprintf(stderr, "libHX-strrep: illegal ->type %d\n", mt->type);
			break;
	}
	return buf;
#undef NTYPE
}

//=============================================================================
