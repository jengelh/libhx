/*
	libHX/format.c
	Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2007

	This file is part of libHX. libHX is free software; you can
	redistribute it and/or modify it under the terms of the GNU
	Lesser General Public License as published by the Free Software
	Foundation; however ONLY version 2 of the License. For details,
	see the file named "LICENSE.LGPL2".
*/
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "libHX.h"

struct fmt_entry {
	const void *ptr;
	unsigned int type, release;
};

EXPORT_SYMBOL struct HXbtree *HXformat_init(struct HXoption *kvtab)
{
	struct HXbtree *table;
	table = HXbtree_init(HXBT_MAP | HXBT_CKEY | HXBT_SCMP | HXBT_CID);
	if(table == NULL)
		return NULL;
	return table;
}

EXPORT_SYMBOL void HXformat_free(struct HXbtree *table)
{
	struct fmt_entry *entry;
	const struct HXbtree_node *nd;
	void *trav = HXbtrav_init(table);

	while((nd = HXbtraverse(trav)) != NULL) {
		entry = nd->data;
		if(entry->release)
			free(static_cast(void *, entry->ptr));
	}

	HXbtrav_free(trav);
	HXbtree_free(table);
	return;
}

EXPORT_SYMBOL int HXformat_addk(struct HXbtree *table, const char *key,
    const char *value)
{
	struct fmt_entry *entry;
	void *ret;

	if((entry = malloc(sizeof(*entry))) == NULL)
		return -errno;
	entry->type    = HXTYPE_STRING;
	entry->ptr     = value;
	entry->release = 1;

	ret = HXbtree_add(table, key, entry);
	if(ret == NULL) {
		free(entry);
		return -errno;
	}

	return 1;
}

EXPORT_SYMBOL int HXformat_addp(struct HXbtree *table, const char *key,
    unsigned int ptr_type, const void *ptr)
{
	struct fmt_entry *entry;
	void *ret;

	if((entry = malloc(sizeof(*entry))) == NULL)
		return -errno;
	entry->type    = ptr_type;
	entry->ptr     = ptr;
	entry->release = 0;

	ret = HXbtree_add(table, key, entry);
	if(ret == NULL) {
		free(entry);
		return -errno;
	}

	return 1;
}

/*
EXPORT_SYMBOL int HXformat_sprintf(struct HXbtree *table, char *dest,
    size_t size, const char *fmt, ...)
{
}

EXPORT_SYMBOL int HXformat_fprintf(struct HXbtree *table, FILE *filp,
    const char *str, ...)
{
}
*/

//=============================================================================
