#ifndef _LIBHX_DEFS_H
#define _LIBHX_DEFS_H 1

#ifndef __cplusplus
	/* N.B. signed_cast<> does not exist in C++. */
#	define __signed_cast_compatible(a, b) \
		__builtin_choose_expr( \
			__builtin_types_compatible_p(b, const char *) || \
			__builtin_types_compatible_p(b, const signed char *) || \
			__builtin_types_compatible_p(b, const unsigned char *), \
			/* if src has a const qualifier */ \
			__builtin_types_compatible_p(a, const char *) || \
			__builtin_types_compatible_p(a, const signed char *) || \
			__builtin_types_compatible_p(a, const unsigned char *), \
			/* and if it has none... */ \
			__builtin_types_compatible_p(a, char *) || \
			__builtin_types_compatible_p(a, signed char *) || \
			__builtin_types_compatible_p(a, unsigned char *) \
		)

#	if defined(__GNUC__) && defined(HXDEV_EXT_CAST)
#		ifndef signed_cast
#			define signed_cast(type, expr) ({ \
				BUILD_BUG_ON(!__signed_cast_compatible(typeof(type), typeof(expr))); \
				(type)(expr); \
			})
#		endif

		/*
		 * This one may cause shadow warnings when nested, or compile
		 * errors when used in declarations, and hence is normally
		 * disabled.
		 */
#		ifndef static_cast
#			define static_cast(type, expr) ({ \
				if (0) { typeof(type) __p __attribute__((unused)) = (expr); } \
				(type)(expr); \
			})
#		endif
#	endif

#	ifndef signed_cast
#		define signed_cast(type, expr)      ((type)(expr))
#	endif
#	ifndef static_cast
#		define static_cast(type, expr)      ((type)(expr))
#	endif
#	ifndef const_cast
#		define const_cast(type, expr)       ((type)(expr))
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
