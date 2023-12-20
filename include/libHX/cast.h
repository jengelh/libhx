#ifndef _LIBHX_CAST_H
#define _LIBHX_CAST_H 1

#ifndef BUILD_BUG_ON_EXPR
#	define BUILD_BUG_ON_EXPR(condition) (sizeof(char[1 - 2 * !!(condition)]) - 1)
#endif
#ifndef BUILD_BUG_ON
#	define BUILD_BUG_ON(condition) ((void)BUILD_BUG_ON_EXPR(condition))
#endif
#ifdef __cplusplus
#	ifndef const_cast
#		define const_cast(T, x) const_cast<T>(x)
#	endif
#	ifndef static_cast
#		define static_cast(T, x) static_cast<T>(x)
#	endif
#	define const_cast1(type, expr)      const_cast<type>(expr)
#	define const_cast2(type, expr)      const_cast<type>(expr)
#	define const_cast3(type, expr)      const_cast<type>(expr)

template<typename new_type>
static __inline__ new_type signed_cast(const char *expr)
{
	return reinterpret_cast<new_type>(expr);
}

template<typename new_type>
static __inline__ new_type signed_cast(const signed char *expr)
{
	return reinterpret_cast<new_type>(expr);
}

template<typename new_type>
static __inline__ new_type signed_cast(const unsigned char *expr)
{
	return reinterpret_cast<new_type>(expr);
}

template<typename new_type>
static __inline__ new_type signed_cast(char *expr)
{
	return reinterpret_cast<new_type>(expr);
}

template<typename new_type>
static __inline__ new_type signed_cast(signed char *expr)
{
	return reinterpret_cast<new_type>(expr);
}

template<typename new_type>
static __inline__ new_type signed_cast(unsigned char *expr)
{
	return reinterpret_cast<new_type>(expr);
}

#else
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
			__builtin_types_compatible_p(a, const char *) || \
			__builtin_types_compatible_p(a, const signed char *) || \
			__builtin_types_compatible_p(a, const unsigned char *) || \
			__builtin_types_compatible_p(a, char *) || \
			__builtin_types_compatible_p(a, signed char *) || \
			__builtin_types_compatible_p(a, unsigned char *) \
		)

#	if defined(__GNUC__) && !defined(__clang__) && !defined(signed_cast)
#		define signed_cast(type, expr) ({ \
			BUILD_BUG_ON(!__signed_cast_compatible(__typeof__(type), __typeof__(expr))); \
			(type)(expr); \
		})
#	endif
#	if defined(__GNUC__) && !defined(__clang__) && !defined(static_cast)
#		define static_cast(type, expr) \
			((struct { type x; }){(expr)}.x)
#	endif
#	if defined(__GNUC__) && !defined(__clang__) && !defined(const_cast1)
		/*
		 * The idea starts with (in abstract notation)
		 * 	typeof deref typeof expr
		 * To deref something, we need an object, which we can get by
		 * creating a temporary aggregate, such as a union, of which
		 * the member is accessed and dereferenced.
		 * 	*(union { __typeof__(expr) x; }){init}.x
		 * union has two nice properties:
		 * - with an additional dummy member, we do not have to
		 *   initialize x according to its type, which, if expr is
		 *   an array type, may want extra braces.
		 * - and with that dummy member, we also avoid the ugly
		 *   "literal 0 is implicitly convertible to a pointer".
		 * Unfortunately, this all requires C99 compound initializers.
		 * That's ok - gcc and clang only treat it as a warning even
		 * under strict C89 - and if you still force strict C89 on
		 * yourself, you have a lot to answer for either way.
		 */
#		define __const_cast_strip(ptrs, expr) \
			__typeof__(ptrs(union { int z; __typeof__(expr) x; }){0}.x)
#		define __const_cast_p(ptrs, new_type, expr) ((new_type)( \
			(expr) + \
			BUILD_BUG_ON_EXPR(!__builtin_types_compatible_p(__const_cast_strip(ptrs, expr), __const_cast_strip(ptrs, new_type))) \
		))
#		define const_cast1(new_type, expr) __const_cast_p(*, new_type, expr)
#		define const_cast2(new_type, expr) __const_cast_p(**, new_type, expr)
#		define const_cast3(new_type, expr) __const_cast_p(***, new_type, expr)
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
#	ifndef const_cast1
#		define const_cast1(type, expr)      ((type)(expr))
#		define const_cast2(type, expr)      ((type)(expr))
#		define const_cast3(type, expr)      ((type)(expr))
#	endif
#	ifndef reinterpret_cast
#		define reinterpret_cast(type, expr) ((type)(expr))
#	endif
#endif

#endif /* _LIBHX_CAST_H */
