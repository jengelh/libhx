#ifndef _LIBHX_DEFS_H
#define _LIBHX_DEFS_H 1

#include <libHX/cast.h>
#ifdef __cplusplus
#	define HXsizeof_member(type, member) sizeof(type::member)
#	if __cplusplus >= 201100L
#		define HXtypeof_member(type, member) decltype(type::member)
#	else
#		define HXtypeof_member(type, member) __typeof__(type::member))
#	endif
#	ifndef containerof
#		include <cstddef>
#		include <type_traits>
namespace {
template<typename Dst, typename Src> static inline auto containerof_cxx(Src *var, size_t ofs)
{
	using K  = typename std::is_const<typename std::remove_pointer<Src>::type>;
	using Ch = typename std::conditional<K::value, const char, char>::type;
	using D2 = typename std::conditional<K::value, const Dst, Dst>::type;
	return reinterpret_cast<D2 *>(reinterpret_cast<Ch *>(var) - ofs);
}
}
#		define containerof(var, D1, member) containerof_cxx<D1>(var, offsetof(D1, member))
#	endif
#else
#	define HXsizeof_member(type, member) sizeof(((type *)NULL)->member)
#	define HXtypeof_member(type, member) __typeof__(((type *)NULL)->member)
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
#define HXSIZEOF_UNITSEC64 sizeof("584542046089y11months2weeks2d23h59min59s")

#define __HX_STRINGIFY_EXPAND(s) #s
#define HX_STRINGIFY(s) __HX_STRINGIFY_EXPAND(s)

#ifndef container_of
#	define container_of(v, s, m) containerof((v), s, m)
#endif

#if !defined(_WIN32) || (defined(__USE_MINGW_ANSI_STDIO) && __USE_MINGW_ANSI_STDIO + 0 > 0)
	/*
	 * Sufficiently old versions of the VC runtime do not even support %ll.
	 */
#	define HX_LONGLONG_FMT "ll"
#	define HX_SIZET_FMT "z"
#else
#	define HX_LONGLONG_FMT "I64"
#	define HX_SIZET_FMT "I"
#endif

#endif /* _LIBHX_DEFS_H */
