/*
 *	libHX/opt.c
 *	Copyright Jan Engelhardt, 2002-2011
 *
 *	This file is part of libHX. libHX is free software; you can
 *	redistribute it and/or modify it under the terms of the GNU Lesser
 *	General Public License as published by the Free Software Foundation;
 *	either version 2.1 or (at your option) any later version.
 */
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX/ctype_helper.h>
#include <libHX/deque.h>
#include <libHX/map.h>
#include <libHX/misc.h>
#include <libHX/option.h>
#include <libHX/string.h>
#include "internal.h"

/* Definitions */
#define C_OPEN  '('
#define C_CLOSE ')'
#define NTYPE_S(con, tpx) NTYPE((con), tpx, strtol)
#define NTYPE_U(con, tpx) NTYPE((con), tpx, strtoul)

#define NTYPE(con, tpx, func) case (con): { \
	tpx *p, v = (func)(cbi->data, NULL, 0); \
	if ((p = opt->ptr) != NULL) { \
		if (opt->type & HXOPT_NOT) \
			v = ~v; \
		switch (opt->type & HXOPT_LOPMASK2) { \
			case 0:         *p  = v; break; \
			case HXOPT_OR:  *p |= v; break; \
			case HXOPT_AND: *p &= v; break; \
			case HXOPT_XOR: *p ^= v; break; \
			default: \
				fprintf(stderr, "libHX-opt: illegal " \
				        "combination of logical op mask\n"); \
				break; \
		} \
	} \
	cbi->data_long = v; \
	break; \
}

#define SCREEN_WIDTH 80 /* fine, popt also has it hardcoded */

enum {

	W_NONE    = 0,
	W_SPACE   = 1 << 0,
	W_BRACKET = 1 << 1,
	W_ALT     = 1 << 2,
	W_EQUAL   = 1 << 3,

	HXOPT_LOPMASK2 = HXOPT_OR | HXOPT_AND | HXOPT_XOR,
	HXOPT_LOPMASK  = HXOPT_LOPMASK2 | HXOPT_NOT,
	HXOPT_TYPEMASK = 0x1F, /* 5 bits */
};

/**
 * HX_getopt_error - internal option parser error codes
 * %HXOPT_E_LONG_UNKNOWN:	unknown long option
 * %HXOPT_E_LONG_TAKESVOID:	long option was used with an arg (--long=arg)
 * %HXOPT_E_LONG_MISSING:	long option requires an argument
 * %HXOPT_E_SHORT_UNKNOWN:	unknown short option
 * %HXOPT_E_SHORT_MISSING:	short option requires an argument
 * %HXOPT_E_AMBIG_PREFIX:	used abbreviated long option but had multiple results
 */
enum {
	HXOPT_E_LONG_UNKNOWN = 1,
	HXOPT_E_LONG_TAKESVOID,
	HXOPT_E_LONG_MISSING,
	HXOPT_E_SHORT_UNKNOWN,
	HXOPT_E_SHORT_MISSING,
	HXOPT_E_AMBIG_PREFIX,
};

/**
 * HX_getopt_state - internal option parser states
 * %HXOPT_S_NORMAL:	base state, options accepted
 * %HXOPT_S_SHORT:	a short option has been seen
 * %HXOPT_S_TWOLONG:	a long option has been seen
 * %HXOPT_S_LONG:	a long option and its argument have been seen
 * %HXOPT_S_TERMINATED:	options closed, all remaining args are to be copied
 */
enum HX_getopt_state {
	HXOPT_S_NORMAL = 0,
	HXOPT_S_SHORT,
	HXOPT_S_TWOLONG,
	HXOPT_S_LONG,
	HXOPT_S_TERMINATED,
};

/**
 * %HXOPT_I_ASSIGN:	call do_assign
 * %HXOPT_I_ADVARG:	advance to next argument in @opt
 * %HXOPT_I_ADVARG2:	advance by two arguments in @opt
 * %HXOPT_I_ADVCHAR:	advance to next character in @cur
 * %HXOPT_I_ERROR:	HXoption error
 */
enum {
	HXOPT_I_ASSIGN  = 1 << 3,
	HXOPT_I_ADVARG  = 1 << 4,
	HXOPT_I_ADVARG2 = 1 << 5,
	HXOPT_I_ADVCHAR = 1 << 6,
	HXOPT_I_ERROR   = 1 << (sizeof(int) * CHAR_BIT - 2),

	HXOPT_I_MASK    = HXOPT_I_ADVARG | HXOPT_I_ADVARG2 | HXOPT_I_ADVCHAR |
	                  HXOPT_I_ASSIGN | HXOPT_I_ERROR,
};

/**
 * struct HX_getopt_vars - option parser working variable set
 * @arg0:	saved program name
 * @remaining:	list of extracted non-options
 * @cbi:	callback info
 * @flags:	flags setting the behavior for HX_getopt
 */
struct HX_getopt_vars {
	const char *arg0;
	struct HXdeque *remaining;
	struct HXoptcb cbi;
	unsigned int flags;
};

struct HXoption HXopt_ambig_prefix;

static bool posix_me_harder(void)
{
	const char *s;
	char *end;
	int res;

	s = getenv("POSIXLY_CORRECT");
	if (s == NULL || *s == '\0')
		return false;
	res = strtol(s, &end, 0);
	if (end != s)
		/* number */
		return res;
	return true; /* non-empty string */
}

static void do_assign(struct HXoptcb *cbi, const char *arg0)
{
	const struct HXoption *opt = cbi->current;

	switch (opt->type & HXOPT_TYPEMASK) {
	case HXTYPE_NONE: {
		if (opt->ptr != NULL) {
			int *p = opt->ptr;
			if (opt->type & HXOPT_INC)      ++*p;
			else if (opt->type & HXOPT_DEC) --*p;
			else                            *p = 1;
		}
		cbi->data_long = 1;
		break;
	}
	case HXTYPE_VAL:
		*static_cast(int *, opt->ptr) = cbi->data_long = opt->val;
		break;
	case HXTYPE_SVAL:
		*static_cast(const char **, opt->ptr) = cbi->data = opt->uptr;
		break;
	case HXTYPE_BOOL: {
		int *p;
		if ((p = opt->ptr) != NULL)
			*p = strcasecmp(cbi->data, "yes") == 0 ||
			     strcasecmp(cbi->data, "on") == 0 ||
			     strcasecmp(cbi->data, "true") == 0 ||
			     (HX_isdigit(*cbi->data) &&
			     strtoul(cbi->data, NULL, 0) != 0);
		break;
	}
	case HXTYPE_BYTE:
		*static_cast(unsigned char *, opt->ptr) = *cbi->data;
		break;

	NTYPE_U(HXTYPE_UCHAR,  unsigned char);
	NTYPE_S(HXTYPE_CHAR,   char);
	NTYPE_U(HXTYPE_USHORT, unsigned short);
	NTYPE_S(HXTYPE_SHORT,  short);
	NTYPE_U(HXTYPE_UINT,   unsigned int);
	NTYPE_S(HXTYPE_INT,    int);
	NTYPE_U(HXTYPE_ULONG,  unsigned long);
	NTYPE_S(HXTYPE_LONG,   long);
	NTYPE_U(HXTYPE_UINT8,  uint8_t);
	NTYPE_S(HXTYPE_INT8,   int8_t);
	NTYPE_U(HXTYPE_UINT16, uint16_t);
	NTYPE_S(HXTYPE_INT16,  int16_t);
	NTYPE_U(HXTYPE_UINT32, uint32_t);
	NTYPE_S(HXTYPE_INT32,  int32_t);
#ifndef _MSC_VER
	NTYPE(HXTYPE_ULLONG,   unsigned long long, strtoull);
	NTYPE(HXTYPE_LLONG,    long long, strtoll);
	NTYPE(HXTYPE_UINT64,   uint64_t, strtoull);
	NTYPE(HXTYPE_INT64,    int64_t, strtoll);
#endif
	NTYPE(HXTYPE_SIZE_T,   size_t, strtoull);
	case HXTYPE_FLOAT:
		cbi->data_dbl = strtod(cbi->data, NULL);
		if (opt->ptr != NULL)
			*static_cast(float *, opt->ptr) = cbi->data_dbl;
		break;
	case HXTYPE_DOUBLE:
		cbi->data_dbl = strtod(cbi->data, NULL);
		if (opt->ptr != NULL)
			*static_cast(double *, opt->ptr) = cbi->data_dbl;
		break;
	case HXTYPE_STRING:
		if (opt->ptr != NULL)
			*static_cast(char **, opt->ptr) = HX_strdup(cbi->data);
		break;
	case HXTYPE_STRDQ:
		HXdeque_push(opt->ptr, HX_strdup(cbi->data));
		break;
	case HXTYPE_MCSTR:
		if (opt->ptr != NULL)
			HXmc_strcpy(opt->ptr, cbi->data);
		break;
	case HXTYPE_XHELP:
		cbi->data = arg0;
		break;
	default:
		fprintf(stderr, "libHX-opt: illegal type %d\n",
		        opt->type & HXOPT_TYPEMASK);
		break;
	} /* switch */
	if (opt->cb != NULL)
		opt->cb(cbi);
}

static __inline__ const struct HXoption *
lookup_short(const struct HXoption *table, char opt)
{
	for (; table->type != HXTYPE_XSNTMARK; ++table)
		if (table->sh == opt)
			return table;
	return NULL;
}

static const struct HXoption *
lookup_long_pfx(const struct HXoption *table, const char *key)
{
	const struct HXoption *cand = NULL;
	size_t klen = strlen(key);

	for (; table->type != HXTYPE_XSNTMARK; ++table) {
		if (table->ln == NULL)
			continue;
		if (strncmp(table->ln, key, klen) != 0)
			continue;
		/* Prefix match */
		if (table->ln[klen] == '\0')
			return table; /* Exact match */
		if (cand != NULL)
			return &HXopt_ambig_prefix;
		cand = table;
	}
	return cand;
}

static __inline__ const struct HXoption *
lookup_long(const struct HXoption *table, const char *key)
{
	for (; table->type != HXTYPE_XSNTMARK; ++table)
		if (table->ln != NULL && strcmp(table->ln, key) == 0)
			return table;
	return NULL;
}

static __inline__ bool takes_void(unsigned int t)
{
	t &= HXOPT_TYPEMASK;
	return t == HXTYPE_NONE || t == HXTYPE_VAL || t == HXTYPE_SVAL ||
	       t == HXTYPE_XSNTMARK || t == HXTYPE_XHELP;
}

static void opt_to_text(const struct HXoption *opt, char *buf, size_t len,
    unsigned int flags)
{
	const char *alt, *htyp = (opt->htyp != NULL) ? opt->htyp : "ARG";
	size_t i = 0;
	char equ;

	if (flags & W_SPACE)   buf[i++] = ' ';
	if (flags & W_BRACKET) buf[i++] = '['; /* ] */
	if (flags & W_ALT) {
		alt = "|";
		equ = (flags & W_EQUAL) ? '=' : ' ';
	} else {
		alt = ", ";
		equ = '=';
	}

	if (opt->ln == NULL) {
		buf[i++] = '-';
		buf[i++] = opt->sh;
		if (!takes_void(opt->type))
			i += snprintf(buf + i, len - i, " %s", htyp);
	} else if (opt->sh == '\0') {
		if (takes_void(opt->type))
			i += snprintf(buf + i, len - i,
			     "--%s", opt->ln);
		else
			i += snprintf(buf + i, len - i,
			     "--%s=%s", opt->ln, htyp);
	} else {
		if (takes_void(opt->type))
			i += snprintf(buf + i, len - i, "-%c%s--%s",
			     opt->sh, alt, opt->ln);
		else
			i += snprintf(buf + i, len - i, "-%c%s--%s%c%s",
			     opt->sh, alt, opt->ln, equ, htyp);
	}

	if (flags & W_BRACKET)
		buf[i++] = ']';
	buf[i] = '\0';
}

static void print_indent(const char *msg, unsigned int ind, FILE *fp)
{
	size_t rest = SCREEN_WIDTH - ind;
	char *p;

	while (true) {
		if (strlen(msg) < rest) {
			fprintf(fp, "%s", msg);
			break;
		}
		if ((p = HX_strbchr(msg, msg + rest, ' ')) == NULL) {
			fprintf(fp, "%s", msg);
			break;
		}
		fprintf(fp, "%.*s\n%*s", static_cast(unsigned int, p - msg),
		        msg, ind, "");
		msg  = p + 1;
		rest = SCREEN_WIDTH - ind;
	}
	fprintf(fp, "\n");
}

/**
 * HXparse_dequote_int - shell-style argument unescape
 * @o:		input/output string
 * @end:	terminating characters
 *
 * Unescapes a quoted argument, in-place.
 * Returns a pointer to one position after the termination character.
 */
static char *HXparse_dequote_int(char *o, const char *end)
{
	char *i, quot = '\0';
	for (i = o; *i != '\0'; ) {
		if (quot == '\0') {
			switch (*i) {
				case '"':
				case '\'':
					quot = *i++;
					continue;
				case '\\':
					if (*++i != '\0')
						*o++ = *i++;
					continue;
			}
			if (end != NULL && strchr(end, *i) != NULL) {
				*o = '\0';
				return i + 1;
			}
			*o++ = *i++;
			continue;
		}
		if (*i == quot) {
			quot = 0;
			++i;
			continue;
		} else if (*i == '\\') {
			if (*++i != '\0')
				*o++ = *i++;
			continue;
		}
		*o++ = *i++;
	}
	*o = '\0';
	return NULL;
}

/**
 * HXparse_dequote_fmt
 * @s:		Input string
 * @end:	Terminating characters. May be %NULL.
 * @pptr:	Return pointer
 *
 * Dequote a string @s until @end, and return an allocated string that will
 * contain the result, or %NULL on error. @*pptr will then point to the
 * terminating character.
 * Nested %() are honored.
 *
 * (This function is used from format.c. It is here in opt.c to call
 * HXparse_dequote_int.)
 */
hxmc_t *HXparse_dequote_fmt(const char *s, const char *end, const char **pptr)
{
	unsigned int level = 0; /* nesting */
	const char *i;
	char quot = '\0';
	hxmc_t *tmp;

	/* Search for end */
	for (i = s; *i != '\0'; ) {
		if (quot == '\0') {
			switch (*i) {
				case '"':
				case '\'':
					quot = *i++;
					continue;
				case '\\':
					if (i[1] != '\0')
						i += 2;
					continue;
				case C_OPEN:
					++level;
					++i;
					continue;
			}
			if (level == 0 && end != NULL &&
			    strchr(end, *i) != NULL)
				break;
			if (i[0] == C_CLOSE && level > 0)
				--level;
			++i;
			continue;
		}
		if (*i == quot) {
			quot = 0;
			++i;
			continue;
		} else if (*i == '\\') {
			if (*++i != '\0')
				++i;
			continue;
		}
		++i;
	}

	if (pptr != NULL)
		*pptr = i;
	tmp = HXmc_meminit(s, i - s);
	if (tmp == NULL)
		return NULL;
	HXparse_dequote_int(tmp, NULL);
	return tmp;
}

static int HX_getopt_error(int err, const char *key, unsigned int flags)
{
	switch (err) {
	case HXOPT_E_LONG_UNKNOWN:
		if (!(flags & HXOPT_QUIET))
			fprintf(stderr, "Unknown option: %s\n", key);
		return HXOPT_I_ERROR | HXOPT_ERR_UNKN;
	case HXOPT_E_LONG_TAKESVOID:
		if (!(flags & HXOPT_QUIET))
			fprintf(stderr, "Option %s does not take "
			        "any argument\n", key);
		return HXOPT_I_ERROR | HXOPT_ERR_VOID;
	case HXOPT_E_LONG_MISSING:
		if (!(flags & HXOPT_QUIET))
			fprintf(stderr, "Option %s requires an "
			        "argument\n", key);
		return HXOPT_I_ERROR | HXOPT_ERR_MIS;
	case HXOPT_E_SHORT_UNKNOWN:
		if (!(flags & HXOPT_QUIET))
			fprintf(stderr, "Unknown option: -%c\n", *key);
		return HXOPT_I_ERROR | HXOPT_ERR_UNKN;
	case HXOPT_E_SHORT_MISSING:
		if (!(flags & HXOPT_QUIET))
			fprintf(stderr, "Option -%c requires an "
			        "argument\n", *key);
		return HXOPT_I_ERROR | HXOPT_ERR_MIS;
	case HXOPT_E_AMBIG_PREFIX:
		if (!(flags & HXOPT_QUIET))
			fprintf(stderr, "Option %s is ambiguous\n", key);
		return HXOPT_I_ERROR | HXOPT_ERR_AMBIG;
	}
	return HXOPT_I_ERROR;
}

static int HX_getopt_twolong(const char *const *opt,
    struct HX_getopt_vars *par)
{
	const char *key = opt[0], *value = opt[1];

	par->cbi.current = lookup_long_pfx(par->cbi.table, key + 2);
	if (par->cbi.current == &HXopt_ambig_prefix)
		return HX_getopt_error(HXOPT_E_AMBIG_PREFIX, key, par->flags);
	if (par->cbi.current == NULL) {
		if (par->flags & HXOPT_PTHRU) {
			char *tmp = HX_strdup(key);
			if (tmp == NULL)
				return -errno;
			if (HXdeque_push(par->remaining, tmp) == NULL) {
				free(tmp);
				return -errno;
			}
			return HXOPT_S_NORMAL | HXOPT_I_ADVARG;
		}
		return HX_getopt_error(HXOPT_E_LONG_UNKNOWN, key, par->flags);
	}

	par->cbi.flags = HXOPTCB_BY_LONG;
	if (takes_void(par->cbi.current->type)) {
		par->cbi.data = NULL;
		return HXOPT_S_NORMAL | HXOPT_I_ASSIGN | HXOPT_I_ADVARG;
	} else if (par->cbi.current->type & HXOPT_OPTIONAL) {
		/* Rule: take arg if next thing is not-null, not-option. */
		if (value == NULL || *value != '-' ||
		    (value[0] == '-' && value[1] == '\0')) {
			/* --file -, --file bla */
			par->cbi.data = value;
			return HXOPT_S_NORMAL | HXOPT_I_ASSIGN | HXOPT_I_ADVARG2;
		} else {
			/* --file --another --file -- endofoptions */
			par->cbi.data = NULL;
			return HXOPT_S_NORMAL | HXOPT_I_ASSIGN | HXOPT_I_ADVARG;
		}
	} else {
		if (value == NULL)
			return HX_getopt_error(HXOPT_E_LONG_MISSING, key, par->flags);
		par->cbi.data = value;
		return HXOPT_S_NORMAL | HXOPT_I_ASSIGN | HXOPT_I_ADVARG2;
	}
}

static int HX_getopt_long(const char *cur, struct HX_getopt_vars *par)
{
	int ret;
	char *key, *value;

	key = HX_strdup(cur);
	if (key == NULL)
		return -errno;

	value = strchr(key, '=');
	*value++ = '\0';
	par->cbi.current = lookup_long_pfx(par->cbi.table, key + 2);
	if (par->cbi.current == &HXopt_ambig_prefix) {
		ret = HX_getopt_error(HXOPT_E_AMBIG_PREFIX, key, par->flags);
		free(key);
		return ret;
	}
	if (par->cbi.current == NULL) {
		if (par->flags & HXOPT_PTHRU) {
			/* Undo nuke of '=' and reuse alloc */
			value[-1] = '=';
			if (HXdeque_push(par->remaining, key) == NULL) {
				free(key);
				return -errno;
			}
			return HXOPT_S_NORMAL | HXOPT_I_ADVARG;
		}
		ret = HX_getopt_error(HXOPT_E_LONG_UNKNOWN, key, par->flags);
		free(key);
		return ret;
	}
	/*
	 * @value is always non-NULL when entering
	 * %HXOPT_S_LONG, so no need to check for !takes_void.
	 */
	if (takes_void(par->cbi.current->type)) {
		ret = HX_getopt_error(HXOPT_E_LONG_TAKESVOID, key, par->flags);
		free(key);
		return ret;
	}

	par->cbi.flags    = HXOPTCB_BY_LONG;
	par->cbi.data     = value;
	/* Not possible to use %HXOPT_I_ASSIGN due to transience of @key. */
	do_assign(&par->cbi, par->arg0);
	free(key);
	return HXOPT_S_NORMAL | HXOPT_I_ADVARG;
}

static int HX_getopt_short(const char *const *opt, const char *cur,
    struct HX_getopt_vars *par)
{
	char op = *cur;

	if (op == '\0')
		return HXOPT_S_NORMAL | HXOPT_I_ADVARG;

	par->cbi.current = lookup_short(par->cbi.table, op);
	if (par->cbi.current == NULL) {
		if (par->flags & HXOPT_PTHRU) {
			/*
			 * @cur-1 is always valid: it is either the previous
			 * char, or it is '-'.
			 */
			char *buf = HX_strdup(cur - 1);
			if (buf != NULL)
				*buf = '-';
			if (HXdeque_push(par->remaining, buf) == NULL) {
				free(buf);
				return -errno;
			}
			return HXOPT_S_NORMAL | HXOPT_I_ADVARG;
		}
		return HX_getopt_error(HXOPT_E_SHORT_UNKNOWN, &op, par->flags);
	}

	par->cbi.flags = HXOPTCB_BY_SHORT;
	if (takes_void(par->cbi.current->type)) {
		/* -A */
		par->cbi.data = NULL;
		return HXOPT_S_SHORT | HXOPT_I_ASSIGN | HXOPT_I_ADVCHAR;
	} else if (cur[1] != '\0') {
		/* -Avalue */
		par->cbi.data = cur + 1;
		return HXOPT_S_NORMAL | HXOPT_I_ASSIGN | HXOPT_I_ADVARG;
	}

	cur = *++opt;
	if (par->cbi.current->type & HXOPT_OPTIONAL) {
		if (cur == NULL || *cur != '-' ||
		    (cur[0] == '-' && cur[1] == '\0')) {
			/* -f - -f bla */
			par->cbi.data = cur;
			return HXOPT_S_NORMAL | HXOPT_I_ASSIGN | HXOPT_I_ADVARG2;
		} else {
			/* -f -a-file --another --file -- endofoptions */
			par->cbi.data = NULL;
			return HXOPT_S_NORMAL | HXOPT_I_ASSIGN | HXOPT_I_ADVARG;
		}
	} else {
		/* -A value */
		if (cur == NULL)
			return HX_getopt_error(HXOPT_E_SHORT_MISSING, &op, par->flags);
		par->cbi.data = cur;
		return HXOPT_S_NORMAL | HXOPT_I_ASSIGN | HXOPT_I_ADVARG2;
	}
}

static int HX_getopt_term(const char *cur, const struct HX_getopt_vars *par)
{
	char *tmp = HX_strdup(cur);
	if (tmp == NULL)
		return -errno;
	if (HXdeque_push(par->remaining, tmp) == NULL) {
		free(tmp);
		return -errno;
	}
	return HXOPT_S_TERMINATED | HXOPT_I_ADVARG;
}

static int HX_getopt_normal(const char *cur, const struct HX_getopt_vars *par)
{
	if (cur[0] == '-' && cur[1] == '\0') {
		/* Note to popt developers: A single dash is NOT an option! */
		HXdeque_push(par->remaining, HX_strdup(cur));
		return HXOPT_S_NORMAL | HXOPT_I_ADVARG;
	}
	if (cur[0] == '-' && cur[1] == '-' && cur[2] == '\0') {
		/*
		 * Double dash. If passthrough is on, "--" must be copied into
		 * @remaining. This is done in the next round.
		 */
		if (!(par->flags & HXOPT_PTHRU))
			return HXOPT_S_TERMINATED | HXOPT_I_ADVARG;
		return HXOPT_S_TERMINATED;
	}
	if (cur[0] == '-' && cur[1] == '-') { /* long option */
		if (strchr(cur + 2, '=') == NULL)
			return HXOPT_S_TWOLONG;
		/* Single argument long option: --long=arg */
		return HXOPT_S_LONG;
	}
	if (cur[0] == '-')
		/* Short option(s) - one or more(!) */
		return HXOPT_S_SHORT | HXOPT_I_ADVCHAR;
	if (par->flags & HXOPT_RQ_ORDER)
		/* POSIX: first non-option implies option termination */
		return HXOPT_S_TERMINATED;
	cur = HX_strdup(cur);
	if (cur == NULL || HXdeque_push(par->remaining, cur) == NULL)
		return -errno;
	return HXOPT_S_NORMAL | HXOPT_I_ADVARG;
}

EXPORT_SYMBOL int HX_getopt(const struct HXoption *table, int *argc,
    const char ***argv, unsigned int flags)
{
	struct HX_getopt_vars ps;
	const char **opt = *argv;
	int state = HXOPT_S_NORMAL;
	int ret = HXOPT_ERR_SUCCESS;
	unsigned int argk;
	const char *cur;

	memset(&ps, 0, sizeof(ps));
	ps.remaining = HXdeque_init();
	if (ps.remaining == NULL)
		goto out;
	ps.flags = flags;
	ps.arg0  = **argv;
	ps.cbi.table = table;

	if (*opt != NULL) {
		/* put argv[0] back */
		char *arg = HX_strdup(*opt++);
		if (arg == NULL)
			goto out_errno;
		if (HXdeque_push(ps.remaining, arg) == NULL) {
			free(arg);
			goto out_errno;
		}
	}

	if (posix_me_harder())
		ps.flags |= HXOPT_RQ_ORDER;

	for (cur = *opt; cur != NULL; ) {
		if (state == HXOPT_S_TWOLONG)
			state = HX_getopt_twolong(opt, &ps);
		else if (state == HXOPT_S_LONG)
			state = HX_getopt_long(cur, &ps);
		else if (state == HXOPT_S_SHORT)
			state = HX_getopt_short(opt, cur, &ps);
		else if (state == HXOPT_S_TERMINATED)
			state = HX_getopt_term(cur, &ps);
		else if (state == HXOPT_S_NORMAL)
			state = HX_getopt_normal(cur, &ps);

		if (state < 0) {
			ret = state;
			break;
		}
		if (state & HXOPT_I_ERROR) {
			ret = state & ~HXOPT_I_ERROR;
			break;
		}
		if (state & HXOPT_I_ASSIGN)
			do_assign(&ps.cbi, ps.arg0);
		if (state & HXOPT_I_ADVARG)
			cur = *++opt;
		else if (state & HXOPT_I_ADVARG2)
			cur = *(opt += 2);
		else if (state & HXOPT_I_ADVCHAR)
			++cur;
		state &= ~HXOPT_I_MASK;
	}

 out:
	if (ret == HXOPT_ERR_SUCCESS) {
		const char **nvec = reinterpret_cast(const char **,
		                    HXdeque_to_vec(ps.remaining, &argk));
		if (nvec == NULL)
			goto out_errno;
		if (ps.flags & HXOPT_DESTROY_OLD)
			/*
			 * Only the "true, original" argv is stored on the
			 * stack - the argv that HX_getopt() produces is on
			 * the heap, so the %HXOPT_DESTROY_OLD flag should be
			 * passed when you use passthrough chaining, i.e. all
			 * but the first call to HX_getopt() should have this
			 * set.
			 */
			HX_zvecfree(const_cast2(char **, *argv));

		*argv = nvec;
		if (argc != NULL)
			*argc = argk;
	} else if (ret < 0) {
		if (!(ps.flags & HXOPT_QUIET))
			fprintf(stderr, "%s: %s\n", __func__, strerror(errno));
	} else {
		ps.cbi.data = ps.arg0;
		if (ps.flags & HXOPT_HELPONERR)
			HX_getopt_help(&ps.cbi, stderr);
		else if (ps.flags & HXOPT_USAGEONERR)
			HX_getopt_usage(&ps.cbi, stderr);
	}

	HXdeque_free(ps.remaining);
	return ret;

 out_errno:
	ret = -errno;
	goto out;
}

EXPORT_SYMBOL void HX_getopt_help(const struct HXoptcb *cbi, FILE *nfp)
{
	FILE *fp = (nfp == NULL) ? stderr : nfp;
	const struct HXoption *travp;
	char tmp[84] = {'\0'};
	unsigned int tw = 0;

	HX_getopt_usage(cbi, nfp);

	/* Find maximum indent */
	for (travp = cbi->table; travp->type != HXTYPE_XSNTMARK; ++travp) {
		size_t tl;

		opt_to_text(travp, tmp, sizeof(tmp), W_EQUAL);
		if ((tl = strlen(tmp)) > tw)
			tw = tl;
	}

	/* Print table */
	for (travp = cbi->table; travp->type != HXTYPE_XSNTMARK; ++travp) {
		opt_to_text(travp, tmp, sizeof(tmp), W_NONE);
		fprintf(fp, "  %-*s    ", static_cast(int, tw), tmp);
		if (travp->help == NULL)
			fprintf(fp, "\n");
		else
			print_indent(travp->help, tw + 6, fp);
	}
}

EXPORT_SYMBOL void HX_getopt_help_cb(const struct HXoptcb *cbi)
{
	HX_getopt_help(cbi, stdout);
	exit(EXIT_SUCCESS);
}

EXPORT_SYMBOL void HX_getopt_usage(const struct HXoptcb *cbi, FILE *nfp)
{
	size_t wd, tw = 0;
	FILE *fp = (nfp == NULL) ? stderr : nfp;
	const struct HXoption *travp;
	char tmp[84] = {};
	/* Program name now expected in .data */
	const char *arg0 = cbi->data;

	if (arg0 == NULL || *arg0 == '\0')
		arg0 = "($0)";

	wd = sizeof("Usage:") + strlen(arg0);
	fprintf(fp, "Usage: %s", arg0);

	/* Short-only flags */
	if (wd + 5 > SCREEN_WIDTH) {
		/* 5 is the minimum size for a new starting option, " [-X]" */
		fprintf(fp, "\n     ");
		wd = 6;
	}
	for (travp = cbi->table; travp->type != HXTYPE_XSNTMARK; ++travp) {
		if (!(travp->ln == NULL && travp->sh != '\0' &&
		    takes_void(travp->type)))
			continue;
		if (*tmp == '\0') {
			snprintf(tmp, sizeof(tmp), " [-"); /* ] */
			tw = 3;
		}
		tmp[tw++] = travp->sh;
		if (wd + tw + 1 > SCREEN_WIDTH) {
			tmp[tw++] = /* [ */ ']';
			tmp[tw]   = '\0';
			fprintf(fp, "%s\n      ", tmp);
			wd   = 6;
			*tmp = '\0';
		}
	}
	if (*tmp != '\0') {
		tmp[tw++] = ']';
		tmp[tw]   = '\0';
		wd += fprintf(fp, "%s", tmp);
	}

	/* Any other args */
	for (travp = cbi->table; travp->type != HXTYPE_XSNTMARK; ++travp) {
		if (travp->ln == NULL && travp->sh != '\0' &&
		    takes_void(travp->type))
			continue;

		opt_to_text(travp, tmp, sizeof(tmp),
		            W_SPACE | W_BRACKET | W_ALT);
		if (wd + strlen(tmp) > SCREEN_WIDTH) {
			fprintf(fp, "\n      ");
			wd = 6;
		}
		wd += fprintf(fp, "%s", tmp);
	}

	fprintf(fp, "\n");
}

EXPORT_SYMBOL void HX_getopt_usage_cb(const struct HXoptcb *cbi)
{
	HX_getopt_usage(cbi, stdout);
	exit(EXIT_SUCCESS);
}

static void HX_shconf_break(void *ptr, char *line,
    void (*cb)(void *, const char *, const char *))
{
	char *lp = line, *key, *val;
	HX_chomp(line);

	while (lp != NULL) {
		while (HX_isspace(*lp) || *lp == ';')
			++lp;
		/* Next entry if comment, empty line or no value */
		if (*lp == '#' || *lp == '\0')
			return;
		if (!HX_isalpha(*lp) && *lp != '_')
			/* Variables ought to start with [A-Z_] */
			return;
		key = lp;
		while (HX_isalnum(*lp) || *lp == '_')
			++lp;
		if (*lp != '=')
			/* Variable name contained something not in [A-Z0-9_] */
			return;
		*lp++ = '\0';
		val = lp;

		/* Handle escape codes and quotes, and assign to TAB entry */
		lp = HXparse_dequote_int(val, "\t\n ;");
		(*cb)(ptr, key, val);
	}
}

static void HX_shconf_assign(void *table, const char *key, const char *value)
{
	struct HXoptcb cbi = {
		.table = table,
		.flags = HXOPTCB_BY_LONG,
		.data  = value,
	};

	if ((cbi.current = lookup_long(table, key)) == NULL)
		return;
	do_assign(&cbi, NULL);
}

EXPORT_SYMBOL int HX_shconfig(const char *file, const struct HXoption *table)
{
	hxmc_t *ln = NULL;
	FILE *fp;

	if ((fp = fopen(file, "r")) == NULL)
		return -errno;

	while (HX_getl(&ln, fp) != NULL)
		HX_shconf_break(const_cast(void *,
			static_cast(const void *, table)), ln,
			HX_shconf_assign);

	HXmc_free(ln);
	fclose(fp);
	return 1;
}

static void HX_shconf_assignmp(void *map, const char *key, const char *value)
{
	HXmap_add(map, key, value);
}

EXPORT_SYMBOL struct HXmap *HX_shconfig_map(const char *file)
{
	struct HXmap *map;
	hxmc_t *ln = NULL;
	FILE *fp;

	map = HXmap_init(HXMAPT_DEFAULT, HXMAP_SCKEY | HXMAP_SCDATA);
	if (map == NULL)
		return NULL;

	if ((fp = fopen(file, "r")) == NULL) {
		int saved_errno = errno;
		HXmap_free(map);
		errno = saved_errno;
		return NULL;
	}

	while (HX_getl(&ln, fp) != NULL)
		HX_shconf_break(map, ln, HX_shconf_assignmp);

	HXmc_free(ln);
	fclose(fp);
	return map;
}

EXPORT_SYMBOL int HX_shconfig_pv(const char **path, const char *file,
    const struct HXoption *table, unsigned int flags)
{
	char buf[MAXFNLEN];
	int ret = 0;

	for (; *path != NULL; ++path) {
		int v;
		snprintf(buf, sizeof(buf), "%s/%s", *path, file);
		v = HX_shconfig(buf, table);
		if (v > 0) {
			++ret;
			if (flags & SHCONF_ONE)
				break;
		}
	}

	return ret;
}

EXPORT_SYMBOL void HX_shconfig_free(const struct HXoption *table)
{
	for (; table->ln != NULL; ++table) {
		char **ptr = table->ptr;
		if (table->type == HXTYPE_STRING &&
		    ptr != NULL && *ptr != NULL)
			free(*ptr);
	}
}
