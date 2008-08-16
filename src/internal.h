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

#include "libHX/config.h"
#include "libHX/defs.h"

#ifdef __MINGW32__
#	include "libHX/uxcompat.h"
#endif
#ifdef _MSC_VER
#	include "libHX/uxcompat.h"
#	define snprintf _snprintf
#endif

#ifdef HAVE_VISIBILITY_HIDDEN
#	define EXPORT_SYMBOL __attribute__((visibility("default")))
#else
#	define EXPORT_SYMBOL
#endif

#define MAXFNLEN 256  /* max length for filename buffer */
#define MAXLNLEN 1024 /* max length for usual line */

#endif /* LIBHX_INTERNAL_H */
