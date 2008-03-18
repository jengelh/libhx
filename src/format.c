/*
 *	libHX/format.c
 *	Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2007 - 2008
 *
 *	This file is part of libHX. libHX is free software; you can
 *	redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2 or 3 of the License.
 */
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "libHX.h"

/* Definitions */
#define MAX_KEY_SIZE 256

struct fmt_entry {
	const void *ptr;
	unsigned int type;
};

struct modifier_info {
	void (*transform)(hmc_t **, const char *);
	const char *name;
	unsigned int length, has_arg;
};

struct modifier {
	void (*transform)(hmc_t **, const char *);
	hmc_t *arg;
};

/* Functions */
static inline char *HX_strchr0(const char *, char);
static const char *HXformat_read_modifier_arg(const char *, struct modifier *);
static int HXformat_read_one_modifier(const char **, struct HXdeque *);
static int HXformat_read_modifiers(const char **, struct HXdeque *);
static hmc_t *HXformat_read_key(const char **);
static void HXformat_transform(hmc_t **, struct HXdeque *,
	const struct fmt_entry *);
static void HXformat_xfrm_after(hmc_t **, const char *);
static void HXformat_xfrm_before(hmc_t **, const char *);
static void HXformat_xfrm_ifempty(hmc_t **, const char *);
static void HXformat_xfrm_ifnempty(hmc_t **, const char *);
static void HXformat_xfrm_lower(hmc_t **, const char *);
static void HXformat_xfrm_upper(hmc_t **, const char *);

/* Variables */
static const struct modifier_info modifier_list[] = {
#define E(s) (s), sizeof(s)-1
	{HXformat_xfrm_after,    E("after=\""),    1},
	{HXformat_xfrm_before,   E("before=\""),   1},
	{HXformat_xfrm_ifempty,  E("ifempty=\""),  1},
	{HXformat_xfrm_ifnempty, E("ifnempty=\""), 1},
	{HXformat_xfrm_lower,    E("lower"),       0},
	{HXformat_xfrm_upper,    E("upper"),       0},
	{NULL},
#undef E
};

//-----------------------------------------------------------------------------
EXPORT_SYMBOL struct HXbtree *HXformat_init(void)
{
	struct HXbtree *table;
	table = HXbtree_init(HXBT_MAP | HXBT_CKEY | HXBT_SCMP | HXBT_CID);
	if (table == NULL)
		return NULL;
	return table;
}

EXPORT_SYMBOL void HXformat_free(struct HXbtree *table)
{
	const struct HXbtree_node *node;
	struct fmt_entry *entry;
	void *trav = HXbtrav_init(table);

	while ((node = HXbtraverse(trav)) != NULL) {
		entry = node->data;
		if (entry->type == (HXTYPE_STRING | HXFORMAT_IMMED))
			free(static_cast(void *, entry->ptr));
		free(entry);
	}

	HXbtrav_free(trav);
	HXbtree_free(table);
	return;
}

EXPORT_SYMBOL int HXformat_add(struct HXbtree *table, const char *key,
    const void *ptr, unsigned int ptr_type)
{
	struct fmt_entry *entry;
	void *ret;

	if (strpbrk(key, "\t\n\v ") != NULL || strlen(key) > MAX_KEY_SIZE) {
		fprintf(stderr, "%s: Bogus key \"%s\"\n", __func__, key);
		return -EINVAL;
	}

	if ((entry = malloc(sizeof(*entry))) == NULL)
		return -errno;

	entry->type = ptr_type;
	if (ptr_type == (HXTYPE_STRING | HXFORMAT_IMMED)) {
		if ((entry->ptr = HX_strdup(ptr)) == NULL) {
			free(entry);
			return -errno;
		}
	} else {
		entry->ptr = ptr;
	}

	ret = HXbtree_add(table, key, entry);
	if (ret == NULL) {
		free(entry);
		return -errno;
	}

	return 1;
}

EXPORT_SYMBOL int HXformat_aprintf(const struct HXbtree *table,
    hmc_t **resultp, const char *fmt)
{
	hmc_t *key, *out = hmc_sinit("");
	const struct fmt_entry *entry;
	const struct modifier *mod;
	const char *last, *current;
	struct HXdeque *dq;
	int ret = 0;

	last = current = fmt;
	if ((dq = HXdeque_init()) == NULL)
		return -errno;

	while ((current = HX_strchr0(last, '%')) != NULL) {
		if (current - last > 0)
			hmc_memcat(&out, last, current - last);
		if (*current == '\0')
			break;
		if (*(current+1) != '(' /* ) */) {
			hmc_strcat(&out, "%");
			last = current + 2;
			continue;
		}

		current += 2; /* skip % and opening parenthesis */
		if (HXformat_read_modifiers(&current, dq) < 0)
			goto out;

		key = HXformat_read_key(&current);
		if ((entry = HXbtree_get(table, key)) == NULL) {
			hmc_strcat(&out, "%(");
			hmc_strcat(&out, key);
			hmc_strcat(&out, ")");
		} else {
			HXformat_transform(&out, dq, entry);
		}

		while ((mod = HXdeque_shift(dq)) != NULL)
			if (mod->arg != NULL)
				hmc_free(mod->arg);

		hmc_free(key);
		last = current + 1; /* closing parenthesis */
	}

	HXdeque_free(dq);
	*resultp = out;
	return strlen(out);

 out:
	ret = -errno;
	hmc_free(out);
	HXdeque_free(dq);
	return ret;
}

EXPORT_SYMBOL int HXformat_fprintf(const struct HXbtree *table, FILE *filp,
    const char *fmt)
{
	hmc_t *str;
	int ret;

	if ((ret = HXformat_aprintf(table, &str, fmt)) <= 0)
		return ret;
	errno = 0;
	if (fputs(str, filp) < 0)
		ret = -errno;
	hmc_free(str);
	return ret;
}

EXPORT_SYMBOL int HXformat_sprintf(const struct HXbtree *table, char *dest,
    size_t size, const char *fmt)
{
	hmc_t *str;
	int ret;

	if ((ret = HXformat_aprintf(table, &str, fmt)) < 0)
		return ret;
	if (ret == 0) {
		*dest = '\0';
		return 0;
	}
	strncpy(dest, str, size);
	hmc_free(str);
	return strlen(dest);
}

//-----------------------------------------------------------------------------
static inline char *HX_strchr0(const char *s, char c)
{
	char *ret = strchr(s, c);
	if (ret != NULL)
		return ret;
	return const_cast(char *, &s[strlen(s)]);
}

static const char *HXformat_read_modifier_arg(const char *data,
    struct modifier *m)
{
	const char *quote = strchr(data, '\"');
	const char *brace = strchr(data, /* ( */ ')');

	if (quote == NULL || (brace != NULL && quote > brace)) {
		fprintf(stderr, "%s: Malformed %%() specifier\n", __func__);
		return data;
	}

	m->arg = NULL;
	hmc_memasg(&m->arg, data, quote - data);
	return quote + 1;
}

static int HXformat_read_one_modifier(const char **pcurrent,
    struct HXdeque *dq)
{
	const struct modifier_info *mod_ptr = modifier_list;
	const char *curr = *pcurrent;
	struct modifier mnew, *mnew_ptr;

	while (mod_ptr->name != NULL) {
		if (strncmp(mod_ptr->name, curr, mod_ptr->length) != 0) {
			++mod_ptr;
			continue;
		}

		curr += mod_ptr->length;
		mnew.transform = mod_ptr->transform;
		if (mod_ptr->has_arg)
			curr = HXformat_read_modifier_arg(curr, &mnew);
		else
			mnew.arg = NULL;

		while (isspace(*curr))
			++curr;

		if ((mnew_ptr = HX_memdup(&mnew, sizeof(mnew))) == NULL)
			return -errno;
		HXdeque_unshift(dq, mnew_ptr);
		*pcurrent = curr;
		return 1;
	}

	return 0;
}

static int HXformat_read_modifiers(const char **current, struct HXdeque *dq)
{
	int ret;
	while ((ret = HXformat_read_one_modifier(current, dq)) > 0)
		/* noop */;
	return ret;
}

static hmc_t *HXformat_read_key(const char **pptr)
{
	const char *ptr = *pptr;
	unsigned int idx = 0, len = strlen(ptr);
	hmc_t *ret = NULL;

	while (idx < len && idx < MAX_KEY_SIZE && ptr[idx] != '\0' &&
	    strchr(/* ( */ "\t\n\v )", ptr[idx]) == NULL)
		++idx;

	hmc_memasg(&ret, ptr, idx);
	*pptr = &ptr[idx];
	return ret;
}

static void HXformat_transform(hmc_t **out, struct HXdeque *dq,
    const struct fmt_entry *entry)
{
#define IMM(fmt, type) \
	snprintf(buf, sizeof(buf), (fmt), \
	static_cast(type, static_cast(long, entry->ptr))); \
	break;
#define PTR(fmt, type) \
	snprintf(buf, sizeof(buf), (fmt), \
	*static_cast(const type *, entry->ptr)); \
	break;

	static const char *const tf[] = {"false", "true"};
	char buf[sizeof("18446744073709551616")-1];
	const struct modifier *mod;
	hmc_t *wp = NULL;

	*buf = '\0';
	switch (entry->type) {
		case HXTYPE_STRING:
		case HXTYPE_STRING | HXFORMAT_IMMED:
			hmc_strasg(&wp, entry->ptr);
			break;
		case HXTYPE_STRP:
			hmc_strasg(&wp, *static_cast(const char **, entry->ptr));
			break;
		case HXTYPE_BOOL:
			hmc_strasg(&wp, tf[!!*static_cast(const int *,
			           entry->ptr)]);
			break;
		case HXTYPE_BOOL | HXFORMAT_IMMED:
			hmc_strasg(&wp, tf[entry->ptr != NULL]);
			break;

		case HXTYPE_BYTE:   PTR("%c", unsigned char);
		case HXTYPE_SHORT:  PTR("%hd", short);
		case HXTYPE_USHORT: PTR("%hu", unsigned short);
		case HXTYPE_CHAR:   PTR("%d", char);
		case HXTYPE_UCHAR:  PTR("%u", unsigned char);
		case HXTYPE_INT:    PTR("%d", int);
		case HXTYPE_UINT:   PTR("%u", unsigned int);
		case HXTYPE_LONG:   PTR("%ld", long);
		case HXTYPE_ULONG:  PTR("%lu", unsigned long);
		case HXTYPE_LLONG:  PTR("%lld", long long);
		case HXTYPE_ULLONG: PTR("%llu", unsigned long long);
		case HXTYPE_FLOAT:  PTR("%f", float);
		case HXTYPE_DOUBLE: PTR("%f", double);

		case HXTYPE_CHAR   | HXFORMAT_IMMED: IMM("%d", char);
		case HXTYPE_UCHAR  | HXFORMAT_IMMED: IMM("%u", unsigned char);
		case HXTYPE_SHORT  | HXFORMAT_IMMED: IMM("%hd", short);
		case HXTYPE_USHORT | HXFORMAT_IMMED: IMM("%hu", unsigned short);
		case HXTYPE_INT    | HXFORMAT_IMMED: IMM("%d", int);
		case HXTYPE_UINT   | HXFORMAT_IMMED: IMM("%u", unsigned int);
		case HXTYPE_LONG   | HXFORMAT_IMMED: IMM("%ld", long);
		case HXTYPE_ULONG  | HXFORMAT_IMMED: IMM("%lu", unsigned long);

		default:
			fprintf(stderr, "%s: Illegal type\n", __func__);
			break;
	}

	if (*buf != '\0')
		hmc_strasg(&wp, buf);

	while ((mod = HXdeque_shift(dq)) != NULL)
		mod->transform(&wp, mod->arg);

	hmc_strcat(out, wp);
	hmc_free(wp);
	return;
#undef IMM
#undef PTR
}

static void HXformat_xfrm_after(hmc_t **x, const char *arg)
{
	if (**x != '\0')
		hmc_strcat(x, arg);
	return;
}

static void HXformat_xfrm_before(hmc_t **x, const char *arg)
{
	if (**x != '\0')
		hmc_strpcat(x, arg);
	return;
}

static void HXformat_xfrm_ifempty(hmc_t **val, const char *repl)
{
	if (**val == '\0' && repl != NULL)
		hmc_strasg(val, repl);
	else
		hmc_trunc(val, 0);
}

static void HXformat_xfrm_ifnempty(hmc_t **val, const char *repl)
{
	if (**val != '\0' && repl != NULL)
		hmc_strasg(val, repl);
	else
		hmc_trunc(val, 0);
}

static void HXformat_xfrm_lower(hmc_t **x, const char *arg)
{
	HX_strlower(*x);
	return;
}

static void HXformat_xfrm_upper(hmc_t **x, const char *arg)
{
	HX_strupper(*x);
	return;
}

//=============================================================================
