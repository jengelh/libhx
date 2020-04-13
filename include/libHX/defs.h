#ifndef _LIBHX_DEFS_H
#define _LIBHX_DEFS_H 1

#ifdef __cplusplus
#	define HXsizeof_member(type, member) sizeof(type::member)
#	if __cplusplus >= 201100L
#		define HXtypeof_member(type, member) decltype(type::member)
#	else
#		define HXtypeof_member(type, member) __typeof__(type::member))
#	endif
#	ifndef containerof
#		include <cstddef>
#		define containerof(var, type, member) reinterpret_cast<type *>( \
			reinterpret_cast<char *>(var) - offsetof(type, member))
#	endif
#	ifndef static_cast
#		define static_cast(T, x) static_cast<T>(x)
#	endif

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
#	define HXsizeof_member(type, member) sizeof(((type *)NULL)->member)
#	define HXtypeof_member(type, member) __typeof__(((type *)NULL)->member)
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
#	ifndef containerof
#		include <stddef.h>
#		define containerof(var, type, member) reinterpret_cast(type *, \
			reinterpret_cast(char *, var) - offsetof(type, member))
#	endif
#endif

#if defined(__GNUC__) && !defined(__cplusplus)
	/*
	 * If typeof @a stays the same through a demotion to pointer,
	 * @a cannot be an array.
	 */
#	define __array_size_check(a) BUILD_BUG_ON_EXPR(\
		__builtin_types_compatible_p(__typeof__(a), \
		__typeof__(DEMOTE_TO_PTR(a))))
#else
#	define __array_size_check(a) 0
#endif
#ifndef ARRAY_SIZE
#	define ARRAY_SIZE(x) (sizeof(x) / sizeof(*(x)) + __array_size_check(x))
#endif
#ifndef BUILD_BUG_ON_EXPR
#	define BUILD_BUG_ON_EXPR(condition) (sizeof(char[1 - 2 * !!(condition)]) - 1)
#endif
#ifndef BUILD_BUG_ON
#	define BUILD_BUG_ON(condition) ((void)BUILD_BUG_ON_EXPR(condition))
#endif
#ifndef DEMOTE_TO_PTR
	/*
	 * An alternative approach is also (p+0), but that does not ensure that
	 * @p is a pointer. Since functions "support" infinite dereferencing,
	 * "&*" also works on them.
	 */
#	define DEMOTE_TO_PTR(p) (&*(p))
#endif
#ifndef O_BINARY
#	define O_BINARY 0
#endif
#ifndef S_IRGRP
	/* Can happen in mingw */
#	define S_IRGRP (S_IRUSR >> 3)
#endif
#ifndef S_IWGRP
#	define S_IWGRP (S_IWUSR >> 3)
#endif
#ifndef S_IXGRP
#	define S_IXGRP (S_IXUSR >> 3)
#endif
#ifndef S_IROTH
#	define S_IROTH (S_IRUSR >> 6)
#endif
#ifndef S_IWOTH
#	define S_IWOTH (S_IWUSR >> 6)
#endif
#ifndef S_IXOTH
#	define S_IXOTH (S_IXUSR >> 6)
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

#define HXSIZEOF_Z16 sizeof("-65536")
/* 2^32 and -2^31 have differing length */
#define HXSIZEOF_Z32 sizeof("-4294967296")
/* 2^64 and -2^63 have same length */
#define HXSIZEOF_Z64 sizeof("18446744073709551616")

#define __HX_STRINGIFY_EXPAND(s) #s
#define HX_STRINGIFY(s) __HX_STRINGIFY_EXPAND(s)

#ifndef container_of
#	define container_of(v, s, m) containerof((v), s, m)
#endif

#ifdef _WIN32
	/*
	 * Sufficiently old versions of the VC runtime do not even support %ll.
	 */
#	define HX_LONGLONG_FMT "I64"
#	define HX_SIZET_FMT "I"
#else
#	define HX_LONGLONG_FMT "ll"
#	define HX_SIZET_FMT "z"
#endif

#endif /* _LIBHX_DEFS_H */
