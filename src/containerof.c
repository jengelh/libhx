/*
 *	GCC: To avoid aliasing optimizations, containerof needs to be in a
 *	different translation unit, so that the caller of the function must
 *	assume that the returned value may alias anything.
 */
#include <stddef.h>
#include <libHX/defs.h>
#include "internal.h"

EXPORT_SYMBOL void *HX_containerof(const void *ptr, size_t offset)
{
	return const_cast1(char *, static_cast(const char *, ptr) - offset);
}
