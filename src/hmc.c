/*
	libHX/hmc.c
	Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2002 - 2007

	This file is part of libHX. libHX is free software; you can
	redistribute it and/or modify it under the terms of the GNU
	Lesser General Public License as published by the Free Software
	Foundation; however ONLY version 2 of the License. For details,
	see the file named "LICENSE.LGPL2".
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libHX.h"

#ifndef offsetof
#	define offsetof(type, member) \
		reinterpret_cast(long, &(static_cast(type *, NULL)->member))
#endif
#ifndef containerof
#	define containerof(var, type, member) reinterpret_cast(type *, \
		reinterpret_cast(const char *, var) - offsetof(type, member))
#endif

#define HMC_IDENT 0x200571AF
#define CHECK_IDENT(c) \
	if((c)->id != HMC_IDENT) \
		fprintf(stderr, "libHX-hmc error: not a hmc object!\n");

struct memcont {
	long alloc, length;
	unsigned int id;
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

EXPORT_SYMBOL hmc_t *hmc_minit(const void *ptr, long len)
{
	char *t = NULL;
	return hmc_memasg(&t, ptr, len);
}

EXPORT_SYMBOL hmc_t *hmc_strasg(hmc_t **vp, const char *s)
{
	if(s == NULL) {
		hmc_free(*vp);
		*vp = NULL;
		return NULL;
	}
	return hmc_memasg(vp, s, strlen(s));
}

EXPORT_SYMBOL hmc_t *hmc_memasg(hmc_t **vp, const void *ptr, long len)
{
	struct memcont *ctx;
	if(*vp != NULL) {
		ctx = containerof(*vp, struct memcont, data);
		CHECK_IDENT(ctx);
		if(ctx->alloc < len) {
			ctx = realloc(ctx, sizeof(struct memcont) + len);
			ctx->alloc = len;
		}
	} else {
		ctx = malloc(sizeof(struct memcont) + len);
		ctx->id    = HMC_IDENT;
		ctx->alloc = len;
	}

	if(ptr == NULL) {
		ctx->length  = 0;
		ctx->data[0] = '\0';
		return *vp = ctx->data;
	}

	memcpy(ctx->data, ptr, ctx->length = len);
	ctx->data[len] = '\0';
	return *vp = ctx->data;
}

EXPORT_SYMBOL long hmc_length(hmc_t *vp)
{
	struct memcont *ctx = containerof(vp, struct memcont, data);
	CHECK_IDENT(ctx);
	return ctx->length;
}

EXPORT_SYMBOL hmc_t *hmc_trunc(hmc_t **vp, long len)
{
	struct memcont *ctx = containerof(*vp, struct memcont, data);
	CHECK_IDENT(ctx);
	if(len > ctx->alloc) {
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
	if(s == NULL)
		return *vp;
	return hmc_memcat(vp, s, strlen(s));
}

EXPORT_SYMBOL hmc_t *hmc_memcat(hmc_t **vp, const void *ptr, long len)
{
	struct memcont *ctx = containerof(*vp, struct memcont, data);
	long nl = ctx->length + len;

	CHECK_IDENT(ctx);
	if(nl > ctx->alloc) {
		ctx = realloc(ctx, sizeof(struct memcont) + nl);
		ctx->alloc = nl;
	}
	if(ptr == NULL)
		return *vp = ctx->data;

	memcpy(&ctx->data[ctx->length], ptr, len);
	ctx->length = nl;
	ctx->data[nl] = '\0';
	return *vp = ctx->data;
}

EXPORT_SYMBOL hmc_t *hmc_strpcat(hmc_t **vp, const char *s)
{
	/* Prepend string @s to @*vp */
	if(s == NULL)
		return *vp;
	return hmc_memins(vp, 0, s, strlen(s));
}

EXPORT_SYMBOL hmc_t *hmc_mempcat(hmc_t **vp, const void *ptr, long len)
{
	/* Prepend memory @ptr (of length @len) to @*vp */
	return hmc_memins(vp, 0, ptr, len);
}

EXPORT_SYMBOL hmc_t *hmc_strins(hmc_t **vp, long pos, const char *s)
{
	if(s == NULL)
		return *vp;
	return hmc_memins(vp, pos, s, strlen(s));
}

EXPORT_SYMBOL hmc_t *hmc_memins(hmc_t **vp, long pos, const void *ptr,
  long len)
{
	struct memcont *ctx = containerof(*vp, struct memcont, data);
	long nl = ctx->length + len;

	CHECK_IDENT(ctx);
	if(ctx->alloc < nl) {
		ctx = realloc(ctx, sizeof(struct memcont) + nl);
		ctx->alloc = nl;
	}
	if(ptr == NULL)
		return *vp = ctx->data;

	memmove(&ctx->data[pos + len], &ctx->data[pos], ctx->length - pos);
	memcpy(&ctx->data[pos], ptr, len);
	ctx->length += len;
	ctx->data[ctx->length] = '\0';
	return *vp = ctx->data;
}

EXPORT_SYMBOL hmc_t *hmc_memdel(hmc_t *vp, long pos, long len)
{
	struct memcont *ctx = containerof(vp, struct memcont, data);
	CHECK_IDENT(ctx);
	memmove(&ctx->data[pos], &ctx->data[pos + len],
	        ctx->length - pos - len);
	ctx->length -= len;
	ctx->data[ctx->length] = '\0';
	return ctx->data;
}

EXPORT_SYMBOL void hmc_free(hmc_t *vp)
{
	struct memcont *ctx;
	if(vp == NULL)
		return;
	ctx = containerof(vp, struct memcont, data);
	CHECK_IDENT(ctx);
	free(ctx);
	return;
}

//=============================================================================
