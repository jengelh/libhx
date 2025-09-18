/*
 *	C-string functions
 *	Copyright Jan Engelhardt, 1999-2010
 *
 *	This file is part of libHX. libHX is free software; you can
 *	redistribute it and/or modify it under the terms of the GNU Lesser
 *	General Public License as published by the Free Software Foundation;
 *	either version 2.1 or (at your option) any later version.
 */
#include <errno.h>
#include <limits.h>
#include <math.h> /* nextafter */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX/ctype_helper.h>
#include <libHX/string.h>
#include "internal.h"

/**
 * %HXQUOTE_ACCEPT:	the listed characters are passed through,
 * 			all others need to be quoted
 * %HXQUOTE_REJECT:	the listed characters need to be quoted,
 * 			all others pass through
 */
enum HX_quote_selector {
	HXQUOTE_ALWAYS,
	HXQUOTE_ACCEPT,
	HXQUOTE_REJECT,
};

/**
 * @selector:	whether this rule is accept- or reject-based
 * @qchars:	characters that need (no) quoting
 */
struct HX_quote_rule {
	char selector;
	const char *chars;
};

/* years, months, weeks, days, hours, minutes, seconds, msecs, musecs, nsecs; */
enum period_idx {
	PERIDX_YEARS = 0,
	PERIDX_MONTHS,
	PERIDX_WEEKS,
	PERIDX_DAYS,
	PERIDX_HOURS,
	PERIDX_MIN,
	PERIDX_SEC,
	PERIDX_MSEC,
	PERIDX_MUSEC,
	PERIDX_NSEC,
	PERIDX_MAX,
};

struct HX_unit_desc {
	const char name[8];
	uint8_t len, pidx;
};

static const char HX_hexenc[] = "0123456789ABCDEF";

EXPORT_SYMBOL char *HX_basename(const char *s)
{
	const char *p;
	/* ignore trailing slashes */
	for (p = s + strlen(s) - 1; p >= s && *p == '/'; --p)
		;
	if (p < s)
		/*
		 * String contained only slashes - this must be the root
		 * directory. Since we have the opportunity, rather than
		 * returning "////", just give the cleaned-up "/".
		 */
		return const_cast1(char *, s + strlen(s) - 1);
	if ((p = HX_strbchr(s, p, '/')) != NULL)
		return const_cast1(char *, p + 1);
	return const_cast1(char *, s);
}

EXPORT_SYMBOL char *HX_basename_exact(const char *s)
{
	const char *start, *end;
	char *ret;
	int len;

	if (*s == '\0')
		return HX_strdup(".");
	/* ignore trailing slashes */
	for (end = s + strlen(s) - 1; end >= s && *end == '/'; --end)
		;
	if (end < s)
		/* string consisted of only slashes */
		return HX_strdup("/");

	start = HX_strbchr(s, end, '/');
	if (start == NULL) {
		len = end - s + 1;
		ret = HX_memdup(s, len + 1);
	} else {
		++start;
		len = end - start + 1;
		ret = HX_memdup(start, len + 1);
	}
	if (ret == NULL)
		return NULL;
	ret[len] = '\0';
	return ret;
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

EXPORT_SYMBOL hxmc_t *HX_getl(hxmc_t **ptr, FILE *fp)
{
	/* Read a whole line into a dynamic buffer. */
	char temp[MAXLNLEN];

	if (fgets(temp, sizeof(temp), fp) == NULL)
		return NULL;

	if (*ptr == NULL) {
		*ptr = HXmc_meminit(NULL, 0);
		if (*ptr == NULL)
			return NULL;
	} else {
		HXmc_trunc(ptr, 0);
	}

	do {
		if (HXmc_strcat(ptr, temp) == NULL)
			return *ptr;
		if (strchr(temp, '\n') != NULL)
			break;
	} while (fgets(temp, sizeof(temp), fp) != NULL);

	return *ptr;
}

EXPORT_SYMBOL void *HX_memmem(const void *vspace, size_t spacesize,
    const void *vpoint, size_t pointsize)
{
	const char *space = vspace, *point = vpoint;
	const char *head, *end;
	size_t tailsize;
	char *tail;

	if (pointsize == 0)
		return const_cast1(void *, vspace);
	if (pointsize > spacesize)
		return NULL;

	/* Do a BM-style trailer search and reduce calls to memcmp */
	head = space + (pointsize - 1);
	tail = memchr(head, point[pointsize-1], spacesize - (pointsize - 1));
	if (tail == NULL || pointsize == 1)
		return tail;
	end = space + spacesize;
	do {
		head = tail - pointsize + 1;
		if (memcmp(head, point, pointsize) == 0)
			return const_cast(char *, head);
		++tail;
		tailsize = end - tail;
		tail = memchr(tail, point[pointsize-1], tailsize);
	} while (tail != NULL);
	return NULL;
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
			if (++*cp >= max && max > 0) {
				*cp = max;
				break;
			}
			++wp;
		}
	}

	if (max == 0 || *cp < max)
		max = *cp;
	else if (*cp > max)
		*cp = max;

	ret = malloc(sizeof(char *) * (*cp + 1));
	if (ret == nullptr)
		return nullptr;
	ret[*cp] = NULL;

	{
		char *wp = HX_strdup(str), *bg = wp;
		size_t i = 0;

		while (--max > 0)
			ret[i++] = HX_strdup(strtok_r(nullptr, delim, &wp));

		ret[i++] = HX_strdup(wp);
		free(bg);
	}

	return ret;
}

EXPORT_SYMBOL char **HX_split_inplace(char *s, const char *delim, int *fld, int max)
{
	char **stk;
	const char *p = s;
	int count = 1;

	for (p = strpbrk(p, delim); p != NULL; p = strpbrk(++p, delim))
		if (++count >= max && max > 0) {
			count = max;
			break;
		}

	stk = malloc(sizeof(char *) * (count + 1));
	if (stk == NULL)
		return NULL;
	stk[count] = NULL;
	count = HX_split_fixed(s, delim, count, stk);
	if (fld != NULL)
		*fld = count;
	return stk;
}

EXPORT_SYMBOL int HX_split_fixed(char *s, const char *delim, int max, char **stk)
{
	/*
	 * HX_split_fixed - the "stack split" (we try to avoid using the heap):
	 * Split @s (modifies it, so must be writable!) at @delim with at most
	 * @max fields and putting the results into @stk[0..@max-1].
	 *
	 * Example on @max:
	 *	char *stk[max];
	 *	HX_split_fixed(s, delim, max, stk);
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
	/* Find the last occurrence of @d within @start and (including) @now. */
	while (now >= start)
		if (*now-- == d)
			return const_cast1(char *, ++now);
	return NULL;
}

/**
 * This is the counterpart to strpbrk(). Returns a pointer to the first
 * character not in @accept, or otherwise %NULL.
 */
EXPORT_SYMBOL char *HX_strchr2(const char *s, const char *accept)
{
	size_t seg = strspn(s, accept);

	if (s[seg] == '\0')
		return NULL;
	return const_cast1(char *, s + seg);
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

EXPORT_SYMBOL char *HX_strdup(const char *src)
{
	if (src == NULL)
		return NULL;
	/* return HX_strndup(src, SIZE_MAX); */
	return HX_memdup(src, strlen(src) + 1);
}

EXPORT_SYMBOL char *HX_strlcat(char *dest, const char *src, size_t len)
{
	ssize_t x = len - strlen(dest) - 1;
	if (x <= 0)
		return dest;
	return strncat(dest, src, x);
}

EXPORT_SYMBOL char *HX_strlcpy(char *dest, const char *src, size_t n)
{
	if (n == 0)
		return dest;
	strncpy(dest, src, n);
	dest[n-1] = '\0';
	return dest;
}

EXPORT_SYMBOL char *HX_strlncat(char *dest, const char *src, size_t dlen,
    size_t slen)
{
	ssize_t x = dlen - strlen(dest) - 1;
	if (x <= 0)
		return dest;
	x = ((ssize_t)slen < x) ? (ssize_t)slen : x;
	return strncat(dest, src, x);
}

EXPORT_SYMBOL char *HX_strlower(char *orig)
{
	char *expr;
	for (expr = orig; *expr != '\0'; ++expr)
		*expr = HX_tolower(*expr);
	return orig;
}

EXPORT_SYMBOL size_t HX_strltrim(char *expr)
{
	char *travp;
	size_t diff = 0;
	travp = expr;

	while (HX_isspace(*travp))
		++travp;
	if ((diff = travp - expr) > 0)
		memmove(expr, travp, strlen(travp) + 1);
	return diff;
}

EXPORT_SYMBOL char *HX_stpltrim(const char *p)
{
	while (HX_isspace(*p))
		++p;
	return const_cast1(char *, p);
}

/**
 * Helper for substr() function for dealing with negative off/len values
 * @z:		total length of string
 * @offset:	n>=0 specifies offset from the start,
 * 		n<0 specifies offset from the end
 * @len:	"length"; n>=0 specifies length to copy (from @offset),
 * 		n<0 specifies the byte relative to the end at which to stop
 *
 * @abstart:	(result) absolute start
 * @retval:	(result) absolute length to copy (from *@abstart)
 */
size_t HX_substr_helper(size_t z, long offset, long len, size_t *start)
{
	if (offset >= 0)
		*start = offset;
	else if (offset == LONG_MIN)
		*start = z + -static_cast(size_t, LONG_MIN);
	else
		*start = z >= static_cast(unsigned long, -offset) ? z + offset : z;

	size_t end;
	if (len >= 0)
		end = *start < SIZE_MAX - len ? *start + len : SIZE_MAX;
	else if (len == LONG_MIN)
		end = z + -static_cast(unsigned long, LONG_MIN);
	else
		end = z >= static_cast(unsigned long, -len) ? z + len : 0;
	if (end > z)
		end = z;
	return end > *start ? end - *start : 0;
}

/* supports negative offsets like scripting languages */
EXPORT_SYMBOL char *HX_strmid(const char *expr, long offset, long length)
{
	size_t start = 0, tocopy = HX_substr_helper(strlen(expr), offset, length, &start);
	char *buffer = malloc(tocopy + 1);
	if (buffer == nullptr)
		return NULL;
	memcpy(buffer, &expr[start], tocopy);
	buffer[tocopy] = '\0';
	return buffer;
}

EXPORT_SYMBOL char *HX_strndup(const char *src, size_t size)
{
	char *ret;
	size_t z;

	if (src == NULL)
		return NULL;
	z = strlen(src);
	if (z < size)
		size = z;
	if ((ret = malloc(size + 1)) == NULL)
		return NULL;
	memcpy(ret, src, size);
	ret[size] = '\0';
	return ret;
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

EXPORT_SYMBOL char *HX_strrev(char *s)
{
	size_t i, z = strlen(s)-1, z2 = z / 2;

	for (i = 0; i < z2; ++i) {
		char temp;
		temp = s[i];
		s[i] = s[z-i];
		s[z-i] = temp;
	}

	return s;
}

EXPORT_SYMBOL size_t HX_strrtrim(char *expr)
{
	size_t i = strlen(expr), s = 0;
	while (i-- && HX_isspace(expr[i]))
		++s;
	expr[++i] = '\0';
	return s;
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

static const struct HX_quote_rule HX_quote_rules[] = {
	[HXQUOTE_SQUOTE]  = {HXQUOTE_REJECT, "'\\"},
	[HXQUOTE_DQUOTE]  = {HXQUOTE_REJECT, "\"\\"},
	[HXQUOTE_HTML]    = {HXQUOTE_REJECT, "\"&<>"},
	[HXQUOTE_LDAPFLT] = {HXQUOTE_REJECT, "\n*()\\"},
	[HXQUOTE_LDAPRDN] = {HXQUOTE_REJECT, "\n \"#+,;<=>\\"},
	[HXQUOTE_URIENC]  = {HXQUOTE_ACCEPT, "-.0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz~"},
	[HXQUOTE_SQLSQUOTE] = {HXQUOTE_REJECT, "'"},
	[HXQUOTE_SQLBQUOTE] = {HXQUOTE_REJECT, "`"},
};

/**
 * HX_qsize_bsa - calculate length of statically expanded string (for accepts)
 * @s:		input string
 * @qchars:	characters that need quoting
 * @cost:	quoting cost per quoted character
 *
 * The cost depends on the quote format. Typical values:
 * 	1	when "&" becomes "\&" (programming language-like)
 * 	2	when "&" becomes "\26" (LDAPRDN/HTTPURI-like hex encoding)
 * 	3	when "&" becomes "\x26" (hex encoding for programming)
 */
static size_t
HX_qsize_bsa(const char *s, const char *qchars, unsigned int cost)
{
	const char *p = s;
	size_t n = strlen(s);

	while ((p = HX_strchr2(p, qchars)) != NULL) {
		n += cost;
		++p;
	}
	return n;
}

/**
 * HX_qsize_bsr - calculate length of statically expanded string (for rejects)
 * @s:		input string
 * @qchars:	characters that need quoting
 * @cost:	quoting cost per quoted character
 *
 * Same as for HX_qsize_bsa, but for HXQUOTE_REJECT-type rules.
 */
static size_t
HX_qsize_bsr(const char *s, const char *qchars, unsigned int cost)
{
	const char *p = s;
	size_t n = strlen(s);

	while ((p = strpbrk(p, qchars)) != NULL) {
		n += cost;
		++p;
	}
	return n;
}

static char *HX_quote_backslash(char *dest, const char *src, const char *qc)
{
	char *ret = dest;
	size_t len;

	while (*src != '\0') {
		len = strcspn(src, qc);
		if (len > 0) {
			memcpy(dest, src, len);
			dest += len;
			src  += len;
			if (*src == '\0')
				break;
		}
		*dest++ = '\\';
		*dest++ = *src++;
	}

	*dest = '\0';
	return ret;
}

static char *
HX_quote_sqlbackslash(char *dest, const char *src, const char *trm)
{
	char *ret = dest;
	size_t len;

	while (*src != '\0') {
		len = strcspn(src, trm);
		if (len > 0) {
			memcpy(dest, src, len);
			dest += len;
			src  += len;
			if (*src == '\0')
				break;
		}
		*dest++ = *trm;
		*dest++ = *trm;
		++src;
	}

	*dest = '\0';
	return ret;
}

/**
 * Encode @src into BASE-64 according to RFC 4648 and write result to @dest,
 * which must be of appropriate size, plus one for a trailing NUL.
 */
static char *HX_quote_base64(char *d, const char *s, char x1, char x2)
{
	char a[] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz0123456789??";
	size_t len = strlen(s);
	char *ret = d;

	a[62] = x1;
	a[63] = x2;
	while (len > 0) {
		if (len >= 3) {
			len -= 3;
			d[0] = a[(s[0] & 0xFC) >> 2];
			d[1] = a[((s[0] & 0x03) << 4) | ((s[1] & 0xF0) >> 4)];
			d[2] = a[((s[1] & 0x0F) << 2) | ((s[2] & 0xC0) >> 6)];
			d[3] = a[s[2] & 0x3F];
		} else if (len == 2) {
			len = 0;
			d[0] = a[(s[0] & 0xFC) >> 2];
			d[1] = a[((s[0] & 0x03) << 4) | ((s[1] & 0xF0) >> 4)];
			d[2] = a[(s[1] & 0x0F) << 2];
			d[3] = '=';
		} else if (len == 1) {
			len = 0;
			d[0] = a[(s[0] & 0xFC) >> 2];
			d[1] = a[(s[0] & 0x03) << 4];
			d[2] = '=';
			d[3] = '=';
		}
		s += 3;
		d += 4;
	}
	*d = '\0';
	return ret;
}

static size_t HX_qsize_html(const char *s)
{
	const char *p = s;
	size_t n = strlen(s);

	while ((p = strpbrk(p, HX_quote_rules[HXQUOTE_HTML].chars)) != NULL) {
		switch (*p) {
		/* minus 2: \0 and the original char */
		case '"':
			n += sizeof("&quot;") - 2;
			break;
		case '&':
			n += sizeof("&amp;") - 2;
			break;
		case '<':
		case '>':
			n += sizeof("&lt;") - 2;
			break;
		}
		++p;
	}
	return n;
}

static char *HX_quote_html(char *dest, const char *src)
{
#define put(s) do { \
	memcpy(dest, (s), sizeof(s) - 1); \
	dest += sizeof(s) - 1; \
} while (false);

	char *ret = dest;

	while (*src != '\0') {
		size_t len = strcspn(src, HX_quote_rules[HXQUOTE_HTML].chars);
		if (len > 0) {
			memcpy(dest, src, len);
			dest += len;
			src  += len;
			if (*src == '\0')
				break;
		}
		switch (*src++) {
		case '"': put("&quot;"); break;
		case '&': put("&amp;"); break;
		case '<': put("&lt;"); break;
		case '>': put("&gt;"); break;
		}
	}
	*dest = '\0';
	return ret;
#undef put
}

static char *HX_quote_ldap(char *dest, const char *src, const char *qc)
{
	char *ret = dest;
	size_t len;

	while (*src != '\0') {
		len = strcspn(src, qc);
		if (len > 0) {
			memcpy(dest, src, len);
			dest += len;
			src  += len;
			if (*src == '\0')
				break;
		}
		*dest++ = '\\';
		*dest++ = HX_hexenc[(*src >> 4) & 0x0F];
		*dest++ = HX_hexenc[*src++ & 0x0F];
	}

	*dest = '\0';
	return ret;
}

static char *HX_quote_urlenc(char *dest, const char *src)
{
	char *ret = dest;
	size_t len;

	while (*src != '\0') {
		len = strspn(src, HX_quote_rules[HXQUOTE_URIENC].chars);
		if (len > 0) {
			memcpy(dest, src, len);
			dest += len;
			src  += len;
			if (*src == '\0')
				break;
		}
		*dest++ = '%';
		*dest++ = HX_hexenc[(*src >> 4) & 0x0F];
		*dest++ = HX_hexenc[*src++ & 0x0F];
	}

	*dest = '\0';
	return ret;
}

/**
 * HX_quoted_size -
 * @s:		string to analyze
 * @type:	quoting method
 *
 * Returns the size of the string @s when quoted.
 */
static size_t HX_quoted_size(const char *s, unsigned int type)
{
	switch (type) {
	case HXQUOTE_SQUOTE:
	case HXQUOTE_DQUOTE:
	case HXQUOTE_SQLSQUOTE:
	case HXQUOTE_SQLBQUOTE:
		return HX_qsize_bsr(s, HX_quote_rules[type].chars, 1);
	case HXQUOTE_HTML:
		return HX_qsize_html(s);
	case HXQUOTE_LDAPFLT:
	case HXQUOTE_LDAPRDN:
		return HX_qsize_bsr(s, HX_quote_rules[type].chars, 2);
	case HXQUOTE_BASE64:
	case HXQUOTE_BASE64URL:
	case HXQUOTE_BASE64IMAP:
		return (strlen(s) + 2) / 3 * 4;
	case HXQUOTE_URIENC:
		return HX_qsize_bsa(s, HX_quote_rules[type].chars, 2);
	default:
		return strlen(s);
	}
}

EXPORT_SYMBOL char *HX_strquote(const char *src, unsigned int type,
    char **free_me)
{
	const struct HX_quote_rule *rule = nullptr;
	bool do_quote;
	char *tmp;

	if (type >= _HXQUOTE_MAX) {
		errno = EINVAL;
		return NULL;
	}
	/* If quote_chars is NULL, it is clear all chars are to be encoded. */
	if (type >= ARRAY_SIZE(HX_quote_rules)) {
		do_quote = true;
	} else {
		rule = &HX_quote_rules[type];
		if (rule->selector == HXQUOTE_ALWAYS)
			do_quote = true;
		else if (rule->selector == HXQUOTE_REJECT)
			do_quote = strpbrk(src, rule->chars) != NULL;
		else if (rule->selector == HXQUOTE_ACCEPT)
			do_quote = HX_strchr2(src, rule->chars) != NULL;
		else
			do_quote = false;
	}
	/*
	 * free_me == NULL implies that we always allocate, even if
	 * there is nothing to quote.
	 */
	if (free_me != NULL) {
		free(*free_me);
		*free_me = NULL;
		if (!do_quote)
			return const_cast1(char *, src);
	} else {
		if (!do_quote)
			return HX_strdup(src);
		free_me = &tmp;
	}

	*free_me = malloc(HX_quoted_size(src, type) + 1);
	if (*free_me == NULL)
		return NULL;

	switch (type) {
	case HXQUOTE_SQUOTE:
	case HXQUOTE_DQUOTE:
		return HX_quote_backslash(*free_me, src, rule->chars);
	case HXQUOTE_HTML:
		return HX_quote_html(*free_me, src);
	case HXQUOTE_LDAPFLT:
	case HXQUOTE_LDAPRDN:
		return HX_quote_ldap(*free_me, src, rule->chars);
	case HXQUOTE_BASE64:
		return HX_quote_base64(*free_me, src, '+', '/');
	case HXQUOTE_BASE64URL:
		return HX_quote_base64(*free_me, src, '-', '_');
	case HXQUOTE_BASE64IMAP:
		return HX_quote_base64(*free_me, src, '+', ',');
	case HXQUOTE_URIENC:
		return HX_quote_urlenc(*free_me, src);
	case HXQUOTE_SQLSQUOTE:
	case HXQUOTE_SQLBQUOTE:
		return HX_quote_sqlbackslash(*free_me, src, rule->chars);
	}
	return NULL;
}

EXPORT_SYMBOL char *HX_strupper(char *orig)
{
	char *expr;
	for (expr = orig; *expr != '\0'; ++expr)
		*expr = HX_toupper(*expr);
	return orig;
}

EXPORT_SYMBOL char *HX_unit_size(char *buf, size_t bufsize,
    unsigned long long size, unsigned int divisor, unsigned int cutoff)
{
	static const char unit_names[] = "\0kMGTPEZYRQ";
	unsigned int unit_idx = 0;
	if (divisor == 0)
		divisor = 1000;
	if (cutoff == 0) {
		cutoff = 10000;
		if (cutoff < divisor)
			cutoff = divisor;
	}
	while (unit_idx < ARRAY_SIZE(unit_names) - 1 && size >= cutoff) {
		++unit_idx;
		size /= divisor;
	}
	snprintf(buf, bufsize, "%llu%.1s", size, &unit_names[unit_idx]);
	return buf;
}

static inline unsigned long long p_90(unsigned long long x)
{
	/* Perform x*9/10, but without the risk of overflow. */
	return x - x / 10 - !!(x % 10);
}

EXPORT_SYMBOL char *HX_unit_size_cu(char *buf, size_t bufsize,
    unsigned long long orig_size, unsigned int divisor)
{
	/* No floating point. Take that, coreutils! */
	static const char unit_names[] = "\0kMGTPEZYRQ";
	unsigned int unit_idx = 0, last_rem = 0;
	unsigned long long size = orig_size, gpow = 1, grand_rem;
	if (divisor == 0)
		divisor = 1000;

	while (unit_idx < ARRAY_SIZE(unit_names) - 1 && size >= divisor) {
		++unit_idx;
		last_rem = size % divisor;
		size /= divisor;
		gpow *= divisor;
	}
	if (unit_idx == 0) {
		snprintf(buf, bufsize, "%llu%.1s", size, &unit_names[unit_idx]);
		return buf;
	}
	grand_rem = orig_size - size * gpow;
	if (grand_rem != 0) {
		if (grand_rem > p_90(gpow)) {
			++size;
			last_rem = 0;
		} else {
			last_rem *= 10;
			last_rem /= divisor;
			++last_rem;
			if (last_rem == 10 || (size >= 10 && last_rem > 0)) {
				++size;
				last_rem = 0;
			}
		}
		if (unit_idx < ARRAY_SIZE(unit_names) - 1 && size == divisor) {
			/* ++size from above may brought size to @divisor again */
			++unit_idx;
			size /= divisor;
		}
	}
	if (size >= 10 && last_rem == 0)
		snprintf(buf, bufsize, "%llu%.1s", size, &unit_names[unit_idx]);
	else
		snprintf(buf, bufsize, "%llu.%01u%.1s", size, last_rem, &unit_names[unit_idx]);
	return buf;
}

static unsigned int suffix_power(char u)
{
	switch (HX_toupper(u)) {
	case 'K': return 1;
	case 'M': return 2;
	case 'G': return 3;
	case 'T': return 4;
	case 'P': return 5;
	case 'E': return 6;
	case 'Z': return 7;
	case 'Y': return 8;
	case 'R': return 9;
	case 'Q': return 10;
	default: return 0;
	}
}

EXPORT_SYMBOL double HX_strtod_unit(const char *s, char **out_end, unsigned int exponent)
{
	char *end;
	double q;

	while (HX_isspace(*s))
		++s;
	q = strtod(s, &end);
	if (exponent == 0)
		exponent = 1000;
	if (end == s) {
		if (out_end != nullptr)
			*out_end = end;
		errno = 0;
		return q;
	}
	while (HX_isspace(*end))
		++end;
	unsigned int pwr = suffix_power(*end);
	if (pwr == 0) {
		if (out_end != nullptr)
			*out_end = const_cast(char *, end);
		errno = 0;
		return q;
	}
	if (out_end != nullptr)
		*out_end = const_cast(char *, end + 1);
	errno = 0;
	return q * pow(exponent, pwr);
}

EXPORT_SYMBOL unsigned long long HX_strtoull_unit(const char *s,
    char **out_end, unsigned int exponent)
{
	char *end;
	unsigned long long ipart;
	unsigned int pwr = 0;
	bool neg = false;

	while (HX_isspace(*s))
		++s;
	if (*s == '-') {
		/*
		 * "-5k": While (-5ULL) * 1000 is the same as (-5000ULL) under
		 * modulo arithmetic, the expression `ipart >= ULLONG_MAX /
		 * exponent` depends on seeing the true value (5 rather than
		 * (-5ULL).)
		 */
		neg = true;
		++s;
	}
	errno = 0;
	ipart = strtoull(s, &end, 10);
	if (ipart == ULLONG_MAX && errno == ERANGE)
		return ipart;
	if (*end == '.') {
		double q = HX_strtod_unit(s, out_end, exponent);
		bool lo_ok = q >= nextafter(-static_cast(double, ULLONG_MAX), 0);
		bool hi_ok = q <= nextafter(static_cast(double, ULLONG_MAX), 0);
		if (!hi_ok || !lo_ok) {
			errno = ERANGE;
			return ULLONG_MAX;
		}
		/*
		 * https://eel.is/c++draft/conv.fpint: values unrepresentable
		 * in the target type (such as forcing -5.2f into a uint) is
		 * UB. Thus check for range and apply the negation after the
		 * conversion to ULL.
		 */
		if (q >= static_cast(double, ULLONG_MAX)) {
			errno = ERANGE;
			return ULLONG_MAX;
		}
		unsigned long long r = q;
		errno = 0;
		return neg ? -r : r;
	}
	if (exponent == 0)
		exponent = 1000;
	while (HX_isspace(*end))
		++end;
	pwr = suffix_power(*end);
	if (pwr == 0) {
		if (out_end != nullptr)
			*out_end = end;
		errno = 0;
		return neg ? -ipart: ipart;
	}
	if (out_end != nullptr)
		*out_end = const_cast(char *, end + 1);
	while (pwr-- > 0) {
		if (ipart >= ULLONG_MAX / exponent) {
			errno = ERANGE;
			return ULLONG_MAX;
		}
		ipart *= exponent;
	}
	errno = 0;
	return neg ? -ipart : ipart;
}

static const struct HX_unit_desc txtperiod_utab[] = {
	{"seconds", 7, PERIDX_SEC},
	{"second",  6, PERIDX_SEC},
	{"sec",     3, PERIDX_SEC},
	{"s",       1, PERIDX_SEC},
	{"minutes", 7, PERIDX_MIN},
	{"minute",  6, PERIDX_MIN},
	{"min",     3, PERIDX_MIN},
	{"hours",   5, PERIDX_HOURS},
	{"hour",    4, PERIDX_HOURS},
	{"h",       1, PERIDX_HOURS},
	{"days",    4, PERIDX_DAYS},
	{"day",     3, PERIDX_DAYS},
	{"d",       1, PERIDX_DAYS},
	{"weeks",   5, PERIDX_WEEKS},
	{"week",    4, PERIDX_WEEKS},
	{"months",  6, PERIDX_MONTHS},
	{"month",   5, PERIDX_MONTHS},
	{"years",   5, PERIDX_YEARS},
	{"year",    4, PERIDX_YEARS},
	{"y",       1, PERIDX_YEARS},
	{"msec",    4, PERIDX_MSEC},
	{"ms",      2, PERIDX_MSEC},
	{"µsec",    5, PERIDX_MUSEC},
	{"µs",      3, PERIDX_MUSEC},
	{"nsec",    4, PERIDX_NSEC},
	{"ns",      2, PERIDX_NSEC},
};

static const struct HX_unit_desc iso8601p_dtab[] = {
	{"D", 1, PERIDX_DAYS},
	{"M", 1, PERIDX_MONTHS},
	{"Y", 1, PERIDX_YEARS},
};

static const struct HX_unit_desc iso8601p_ttab[] = {
	{"S", 1, PERIDX_SEC},
	{"M", 1, PERIDX_MIN},
	{"H", 1, PERIDX_HOURS},
};

/* Numbers also used by systemd — the focus is on longterm averages */
#define SECONDS_PER_YEAR 31557600 /* 365.25 days */
#define SECONDS_PER_MONTH 2629800 /* 1/12th of that year = 30.4375 days */
#define NSEC_PER_SECOND 1000000000ULL

static const uint64_t sec_mult[PERIDX_MAX] = {
	[PERIDX_YEARS]  = SECONDS_PER_YEAR,
	[PERIDX_MONTHS] = SECONDS_PER_MONTH,
	[PERIDX_WEEKS]  = 604800,
	[PERIDX_DAYS]   = 86400,
	[PERIDX_HOURS]  = 3600,
	[PERIDX_MIN]    = 60,
	[PERIDX_SEC]    = 1,
	[PERIDX_MSEC]   = 0,
	[PERIDX_MUSEC]  = 0,
	[PERIDX_NSEC]   = 0,
};

static const uint64_t nsec_mult[PERIDX_MAX] = {
	[PERIDX_YEARS]  = NSEC_PER_SECOND * SECONDS_PER_YEAR,
	[PERIDX_MONTHS] = NSEC_PER_SECOND * SECONDS_PER_MONTH,
	[PERIDX_WEEKS]  = NSEC_PER_SECOND * 604800,
	[PERIDX_DAYS]   = NSEC_PER_SECOND * 86400,
	[PERIDX_HOURS]  = NSEC_PER_SECOND * 3600,
	[PERIDX_MIN]    = NSEC_PER_SECOND * 60,
	[PERIDX_SEC]    = NSEC_PER_SECOND,
	[PERIDX_MSEC]   = 1000000,
	[PERIDX_MUSEC]  = 1000,
	[PERIDX_NSEC]   = 1,
};

/**
 * Take a textual period string ("1 minute 5 seconds") and break it down.
 * @s:       input string
 * @utab:    allowed unit suffixes
 * @mtab:    multiplication table
 * @out_end: parsing stopping point
 *
 * Returns an errno.
 */
static unsigned long long HX_strtoull_period(const char *s,
    const struct HX_unit_desc *utab, size_t usize, const uint64_t *mtab,
    size_t msize, char **out_end)
{
	unsigned long long quant = 0;

	while (*s != '\0') {
		while (HX_isspace(*s))
			++s;
		const char *numbegin = s;
		if (*s == '-')
			break;
		char *end = nullptr;
		unsigned long long num = strtoull(s, &end, 10);
		if (num == ULLONG_MAX && errno == ERANGE)
			return num;
		double frac = 0;
		bool have_frac = *end == '.';
		if (have_frac)
			frac = strtod(s, &end);
		if (end == s)
			break;
		s = end;
		while (HX_isspace(*s))
			++s;
		unsigned int i;
		for (i = 0; i < usize; ++i)
			if (strncasecmp(s, utab[i].name, utab[i].len) == 0 &&
			    /* Ensure a word boundary is present */
			    !HX_isalpha(s[utab[i].len]))
				break;
		if (i == usize) {
			if ((!have_frac && num == 0) || (have_frac && frac == 0))
				/* 0 is the same no matter what unit, take it */
				continue;
			s = numbegin;
			break;
		}
		unsigned long long mult = mtab[utab[i].pidx];
		if (have_frac) {
			double v = frac * mult;
			if (v >= static_cast(double, ULLONG_MAX)) {
				/*
				 * As ULLONG_MAX gets promoted to double, its
				 * value may _increase_, therefore here we use
				 * >= and not >.
				 */
				if (out_end != nullptr)
					*out_end = const_cast(char *, numbegin);
				errno = ERANGE;
				return ULLONG_MAX;
			}
			num = v;
		} else {
			if (mult > 0 && num > ULLONG_MAX / mult) {
				if (out_end != nullptr)
					*out_end = const_cast(char *, numbegin);
				errno = ERANGE;
				return ULLONG_MAX;
			}
			num *= mult;
		}
		if (num > ULLONG_MAX - quant) {
			if (out_end != nullptr)
				*out_end = const_cast(char *, numbegin);
			errno = ERANGE;
			return ULLONG_MAX;
		}
		quant += num;
		s += utab[i].len;
	}
	if (out_end != nullptr)
		*out_end = const_cast(char *, s);
	errno = 0;
	return quant;
}

static unsigned long long HX_strtoull_iso8601p_2(const char *s,
    const uint64_t *mtab, size_t msize, char **out_end)
{
	const struct HX_unit_desc *utab = iso8601p_dtab;
	size_t usize = ARRAY_SIZE(iso8601p_dtab);
	unsigned long long quant = 0;

	while (*s != '\0') {
		while (HX_isspace(*s))
			++s;
		const char *numbegin = s;
		if (*s == '-')
			break;
		if (HX_toupper(*s) == 'T') {
			if (utab != iso8601p_dtab)
				break;
			utab = iso8601p_ttab;
			usize = ARRAY_SIZE(iso8601p_ttab);
			++s;
			continue;
		}
		char *end = nullptr;
		unsigned long long num = strtoull(s, &end, 10);
		if (num == ULLONG_MAX && errno == ERANGE)
			return num;
		double frac = 0;
		bool have_frac = *end == '.';
		if (have_frac)
			frac = strtod(s, &end);
		if (end == s)
			break;
		s = end;
		while (HX_isspace(*s))
			++s;
		unsigned int i;
		/* No word boundary check for 8601 mode */
		for (i = 0; i < usize; ++i)
			if (strncmp(s, utab[i].name, utab[i].len) == 0)
				break;
		if (i == usize) {
			if ((!have_frac && num == 0) || (have_frac && frac == 0))
				/* 0 is the same no matter what unit, take it */
				continue;
			s = numbegin;
			break;
		}
		unsigned long long mult = mtab[utab[i].pidx];
		if (have_frac) {
			double v = frac * mult;
			if (v >= static_cast(double, ULLONG_MAX)) {
				/*
				 * As ULLONG_MAX gets promoted to double, its
				 * value may _increase_, therefore here we use
				 * >= and not >.
				 */
				if (out_end != nullptr)
					*out_end = const_cast(char *, numbegin);
				errno = ERANGE;
				return ULLONG_MAX;
			}
			num = v;
		} else {
			if (mult > 0 && num > ULLONG_MAX / mult) {
				if (out_end != nullptr)
					*out_end = const_cast(char *, numbegin);
				errno = ERANGE;
				return ULLONG_MAX;
			}
			num *= mult;
		}
		if (num > ULLONG_MAX - quant) {
			if (out_end != nullptr)
				*out_end = const_cast(char *, numbegin);
			errno = ERANGE;
			return ULLONG_MAX;
		}
		quant += num;
		s += utab[i].len;
	}
	if (out_end != nullptr)
		*out_end = const_cast(char *, s);
	errno = 0;
	return quant;
}

static bool looks_like_iso8601(const char *s)
{
	if (s[0] != 'P')
		return false;
	if (HX_isdigit(s[1]))
		return true;
	if (s[1] == 'T' && HX_isdigit(s[2]))
		return true;
	return false;
}

EXPORT_SYMBOL unsigned long long HX_strtoull_sec(const char *s, char **out_end)
{
	if (looks_like_iso8601(s))
		return HX_strtoull_iso8601p_2(&s[1], sec_mult, ARRAY_SIZE(sec_mult), out_end);
	return HX_strtoull_period(s,
	       txtperiod_utab, ARRAY_SIZE(txtperiod_utab),
	       sec_mult, ARRAY_SIZE(sec_mult), out_end);
}

EXPORT_SYMBOL unsigned long long HX_strtoull_nsec(const char *s, char **out_end)
{
	if (looks_like_iso8601(s))
		return HX_strtoull_iso8601p_2(&s[1], nsec_mult, ARRAY_SIZE(nsec_mult), out_end);
	return HX_strtoull_period(s,
	       txtperiod_utab, ARRAY_SIZE(txtperiod_utab),
	       nsec_mult, ARRAY_SIZE(nsec_mult), out_end);
}

EXPORT_SYMBOL unsigned long long HX_strtoull8601p_sec(const char *s, char **out_end)
{
	if (*s == 'P')
		return HX_strtoull_iso8601p_2(&s[1], sec_mult, ARRAY_SIZE(sec_mult), out_end);
	if (out_end != nullptr)
		*out_end = const_cast(char *, s);
	return 0;
}

EXPORT_SYMBOL char *HX_unit_seconds(char *out, size_t outsize,
    unsigned long long secs, unsigned int flags)
{
	unsigned long long years = 0, months = 0, weeks = 0, days, hours, mins;
	char buf[HXSIZEOF_Z64+4];
	if (flags & HXUNIT_YEARS) {
		years = secs / SECONDS_PER_YEAR;
		secs %= SECONDS_PER_YEAR;
	}
	if (flags & HXUNIT_MONTHS) {
		months = secs / SECONDS_PER_MONTH;
		secs %= SECONDS_PER_MONTH;
	}
	if (flags & HXUNIT_WEEKS) {
		weeks = secs / 604800;
		secs %= 604800;
	}
	days = secs / 86400;
	secs %= 86400;
	hours = secs / 3600;
	secs %= 3600;
	mins = secs / 60;
	secs %= 60;
	*out = '\0';
	if (years > 0) {
		snprintf(buf, ARRAY_SIZE(buf), "%lluy", years);
		HX_strlcat(out, buf, outsize);
	}
	if (months == 1) {
		HX_strlcat(out, "1month", outsize);
	} else if (months > 0) {
		snprintf(buf, ARRAY_SIZE(buf), "%llumonths", months);
		HX_strlcat(out, buf, outsize);
	}
	if (weeks == 1) {
		HX_strlcat(out, "1week", outsize);
	} else if (weeks > 0) {
		snprintf(buf, ARRAY_SIZE(buf), "%lluweeks", weeks);
		HX_strlcat(out, buf, outsize);
	}
	if (days > 0) {
		snprintf(buf, ARRAY_SIZE(buf), "%llud", days);
		HX_strlcat(out, buf, outsize);
	}
	if (hours > 0) {
		snprintf(buf, ARRAY_SIZE(buf), "%lluh", hours);
		HX_strlcat(out, buf, outsize);
	}
	if (mins > 0) {
		snprintf(buf, ARRAY_SIZE(buf), "%llumin", mins);
		HX_strlcat(out, buf, outsize);
	}
	if (secs > 0 ||
	    (years == 0 && months == 0 && weeks == 0 &&
	    days == 0 && hours == 0 && mins == 0)) {
		snprintf(buf, ARRAY_SIZE(buf), "%llus", secs);
		HX_strlcat(out, buf, outsize);
	}
	return out;
}
