/*
 *	libHX/internal.h
 *	Copyright © Jan Engelhardt <jengelh [at] gmx de>, 1999 - 2007
 *
 *	This file is part of libHX. libHX is free software; you can
 *	redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2 or 3 of the License.
 */
#ifndef LIBHX_INTERNAL_H
#define LIBHX_INTERNAL_H 1

#include "config.h"

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

#ifndef O_BINARY
#	define O_BINARY 0
#endif
#define MAXFNLEN 256  /* max length for filename buffer */
#define MAXLNLEN 1024 /* max length for usual line */

#define const_cast(type, expr)       ((type)(expr))
#define static_cast(type, expr)      ((type)(expr))
#define reinterpret_cast(type, expr) ((type)(expr))

#ifndef offsetof
#	define offsetof(type, member) \
		reinterpret_cast(long, &(static_cast(type *, NULL)->member))
#endif
#ifndef containerof
#	define containerof(var, type, member) reinterpret_cast(type *, \
		reinterpret_cast(const char *, var) - offsetof(type, member))
#endif

#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2 * !!(condition)]))

#endif /* LIBHX_INTERNAL_H */
