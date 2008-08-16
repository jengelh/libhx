#ifndef _LIBHX_DEFS_H
#define _LIBHX_DEFS_H 1

#ifndef __cplusplus
#	ifndef const_cast
#		define const_cast(type, expr)       ((type)(expr))
#	endif
#	ifndef static_cast
#		define static_cast(type, expr)      ((type)(expr))
#	endif
#	ifndef reinterpret_cast
#		define reinterpret_cast(type, expr) ((type)(expr))
#	endif
#endif
#ifndef offsetof
#	define offsetof(type, member) \
		reinterpret_cast(long, &(static_cast(type *, NULL)->member))
#endif
#ifndef containerof
#	define containerof(var, type, member) reinterpret_cast(type *, \
		reinterpret_cast(const char *, var) - offsetof(type, member))
#endif
#ifndef ARRAY_SIZE
#	define ARRAY_SIZE(x) (sizeof(x) / sizeof(*(x)))
#endif
#ifndef BUILD_BUG_ON
#	define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2 * !!(condition)]))
#endif
#ifndef O_BINARY
#	define O_BINARY 0
#endif
#ifndef S_IRUGO
#	define S_IRUGO (S_IRUSR | S_IRGRP | S_IROTH)
#endif
#ifndef S_IWUGO
#	define S_IWUGO (S_IWUSR | S_IWGRP | S_IWOTH)
#endif
#ifndef S_IXUGO
#	define S_IXUGO (S_IXUSR | S_IXGRP | S_IXOTH)
#endif
#ifndef S_IRWXUGO
#	define S_IRWXUGO (S_IRUGO | S_IWUGO | S_IXUGO)
#endif

#endif /* _LIBHX_DEFS_H */
