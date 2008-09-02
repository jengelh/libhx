/*
 *	libHX/libHX.h
 *	Copyright © Jan Engelhardt <jengelh [at] gmx de>, 1999 - 2008
 *
 *	This file is part of libHX. libHX is free software; you can
 *	redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2 or 3 of the License.
 */
#ifndef _LIBHX_H
#define _LIBHX_H 20080610

#ifndef __cplusplus
#	include <stdarg.h>
#	include <stdio.h>
#	include <stdlib.h>
#	include <string.h>
#else
#	include <cstdarg>
#	include <cstdio>
#	include <cstdlib>
#	include <cstring>
#endif
#if defined _WIN32
#	include <windows.h>
#else
#	include <dirent.h>
#	include <dlfcn.h>
#	include <unistd.h>
#endif
#ifdef LIBHX_INTERNAL /* only for compiling libHX */
#	include "libHX/internal.h"
#endif

#include <libHX/arbtree.h>
#include <libHX/deque.h>
#include <libHX/misc.h>
#include <libHX/option.h>
#include <libHX/string.h>

#endif /* _LIBHX_H */
