/*
 *	Copyright Jan Engelhardt, 1999-2008
 *
 *	This file is part of libHX. libHX is free software; you can
 *	redistribute it and/or modify it under the terms of the GNU Lesser
 *	General Public License as published by the Free Software Foundation;
 *	either version 2.1 or (at your option) any later version.
 */
#ifndef LIBHX_INTERNAL_H
#define LIBHX_INTERNAL_H 1

#include "config.h"
#include <stdint.h>
#include <libHX/cast.h>
#include <libHX/defs.h>
#include <libHX/string.h>

#ifdef __cplusplus
	/* Only for our dual C/C++ testsuites */
#	define dynamic_cast(type, expr)     dynamic_cast<type>(expr)
#	define signed_cast(type, expr)      signed_cast<type>(expr)
#	define reinterpret_cast(type, expr) reinterpret_cast<type>(expr)
#endif

#ifdef __MINGW32__
#	include "uxcompat.h"
#endif
#ifdef _MSC_VER
#	include "uxcompat.h"
#	define snprintf _snprintf
#endif

#ifdef HAVE_VISIBILITY_HIDDEN
#	define EXPORT_SYMBOL __attribute__((visibility("default")))
#else
#	define EXPORT_SYMBOL
#endif

#define MAXFNLEN 256  /* max length for filename buffer */
#define MAXLNLEN 1024 /* max length for usual line */

#define HXMC_IDENT 0x200571AF
#if !defined(__cplusplus)
#	define nullptr NULL
#endif

struct memcont {
	size_t alloc, length;
	unsigned int id;
	char data[];
};

struct timespec;
struct timeval;

extern hxmc_t *HXparse_dequote_fmt(const char *, const char *, const char **);
extern size_t HX_substr_helper(size_t, long, long, size_t *);

#endif /* LIBHX_INTERNAL_H */
