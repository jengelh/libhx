/*
 *	libHX/hmc.c
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2002 - 2008
 *
 *	This file is part of libHX. libHX is free software; you can
 *	redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2 or 3 of the License.
 */
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libHX.h"

#define HMC_IDENT 0x200571AF
#define CHECK_IDENT(c) \
	if ((c)->id != HMC_IDENT) \
		fprintf(stderr, "libHX-hmc error: not a hmc object!\n");

struct memcont {
	size_t alloc, length;
	unsigned int id;
	/*
	 * Not using data[0] on purpose. With data[1], we may end up enlarging
	 * this struct due to padding, but at least we can always make sure
	 * (with appropriate code) that @data is '\0' terminated, even if it
	 * is a binary blob.
	 */
	char data[1];
};

//-----------------------------------------------------------------------------
EXPORT_SYMBOL hmc_t *hmc_dup(const void *vp)
{
	struct memcont *dst, *src = containerof(vp, struct memcont, data);
	CHECK_IDENT(src);
	dst = HX_memdup(src, sizeof(struct memcont) + src->alloc);
	return dst->data;
}

EXPORT_SYMBOL hmc_t *hmc_sinit(const char *s)
{
	char *t = NULL;
	return hmc_memasg(&t, s, strlen(s));
}

EXPORT_SYMBOL hmc_t *hmc_minit(const void *ptr, size_t len)
{
	char *t = NULL;
	return hmc_memasg(&t, ptr, len);
}

EXPORT_SYMBOL hmc_t *hmc_strasg(hmc_t **vp, const char *s)
{
	if (s == NULL) {
		hmc_free(*vp);
		*vp = NULL;
		return NULL;
	}
	return hmc_memasg(vp, s, strlen(s));
}

EXPORT_SYMBOL hmc_t *hmc_memasg(hmc_t **vp, const void *ptr, size_t len)
{
	struct memcont *ctx;
	if (*vp != NULL) {
		ctx = containerof(*vp, struct memcont, data);
		CHECK_IDENT(ctx);
		if (ctx->alloc < len) {
			ctx = realloc(ctx, sizeof(struct memcont) + len);
			ctx->alloc = len;
		}
	} else {
		ctx = malloc(sizeof(struct memcont) + len);
		ctx->id    = HMC_IDENT;
		ctx->alloc = len;
	}

	if (ptr == NULL) {
		ctx->length  = 0;
		ctx->data[0] = '\0';
		return *vp = ctx->data;
	}

	memcpy(ctx->data, ptr, ctx->length = len);
	ctx->data[len] = '\0';
	return *vp = ctx->data;
}

EXPORT_SYMBOL size_t hmc_length(const hmc_t *vp)
{
	struct memcont *ctx = containerof(vp, struct memcont, data);
	CHECK_IDENT(ctx);
	return ctx->length;
}

EXPORT_SYMBOL hmc_t *hmc_trunc(hmc_t **vp, size_t len)
{
	struct memcont *ctx = containerof(*vp, struct memcont, data);
	CHECK_IDENT(ctx);
	if (len > ctx->alloc) {
		ctx = realloc(ctx, sizeof(struct memcont) + len);
		ctx->alloc = len;
	} else {
		ctx->data[len] = '\0';
		ctx->length = len;
	}
	return ctx->data;
}

EXPORT_SYMBOL hmc_t *hmc_strcat(hmc_t **vp, const char *s)
{
	if (s == NULL)
		return *vp;
	return hmc_memcat(vp, s, strlen(s));
}

EXPORT_SYMBOL hmc_t *hmc_memcat(hmc_t **vp, const void *ptr, size_t len)
{
	struct memcont *ctx = containerof(*vp, struct memcont, data);
	size_t nl = ctx->length + len;

	CHECK_IDENT(ctx);
	if (nl > ctx->alloc) {
		ctx = realloc(ctx, sizeof(struct memcont) + nl);
		ctx->alloc = nl;
	}
	if (ptr == NULL)
		return *vp = ctx->data;

	memcpy(&ctx->data[ctx->length], ptr, len);
	ctx->length = nl;
	ctx->data[nl] = '\0';
	return *vp = ctx->data;
}

EXPORT_SYMBOL hmc_t *hmc_strpcat(hmc_t **vp, const char *s)
{
	/* Prepend string @s to @*vp */
	if (s == NULL)
		return *vp;
	return hmc_memins(vp, 0, s, strlen(s));
}

EXPORT_SYMBOL hmc_t *hmc_mempcat(hmc_t **vp, const void *ptr, size_t len)
{
	/* Prepend memory @ptr (of length @len) to @*vp */
	return hmc_memins(vp, 0, ptr, len);
}

EXPORT_SYMBOL hmc_t *hmc_strins(hmc_t **vp, size_t pos, const char *s)
{
	if (s == NULL)
		return *vp;
	return hmc_memins(vp, pos, s, strlen(s));
}

/*
 * We naturally do not support negative positions like some
 * scripting languages do, hence @pos is unsigned.
 */

EXPORT_SYMBOL hmc_t *hmc_memins(hmc_t **vp, size_t pos, const void *ptr,
    size_t len)
{
	struct memcont *ctx = containerof(*vp, struct memcont, data);
	size_t nl = ctx->length + len;

	CHECK_IDENT(ctx);
	if (ctx->alloc < nl) {
		ctx = realloc(ctx, sizeof(struct memcont) + nl);
		ctx->alloc = nl;
	}
	if (ptr == NULL)
		return *vp = ctx->data;

	memmove(&ctx->data[pos + len], &ctx->data[pos], ctx->length - pos);
	memcpy(&ctx->data[pos], ptr, len);
	ctx->length += len;
	ctx->data[ctx->length] = '\0';
	return *vp = ctx->data;
}

EXPORT_SYMBOL hmc_t *hmc_memdel(hmc_t *vp, size_t pos, size_t len)
{
	struct memcont *ctx = containerof(vp, struct memcont, data);
	CHECK_IDENT(ctx);

	if (pos + len > ctx->length)
		len = ctx->length - pos;

	memmove(&ctx->data[pos], &ctx->data[pos + len],
	        ctx->length - pos - len);
	ctx->length -= len;
	ctx->data[ctx->length] = '\0';
	return ctx->data;
}

EXPORT_SYMBOL void hmc_free(hmc_t *vp)
{
	struct memcont *ctx;
	if (vp == NULL)
		return;
	ctx = containerof(vp, struct memcont, data);
	CHECK_IDENT(ctx);
	free(ctx);
}
