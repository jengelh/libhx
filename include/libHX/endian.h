#ifndef _LIBHX_ENDIAN_H
#define _LIBHX_ENDIAN_H 1
/*
 * See <https://commandcenter.blogspot.com/2012/04/byte-order-fallacy.html>.
 *
 * be16p_to_cpu not optimized on LE before gcc-10.
 * le16p_to_cpu not optimized away on LE before gcc-10.
 * cpu_to_be{16,32,64}p not optimized on LE before gcc-8.
 * cpu_to_le{16,32,64}p not optimized away on LE before gcc-8.
 * cpu_to_le{16,32,64} not optimized away on LE before gcc-8.
 * cpu_to_be16 not optimized on LE before gcc-9.
 * cpu_to_be{32,64} not optimized on LE before gcc-8.
 * be64_to_cpu not optimized on LE before gcc-10.
 */
#include <stdint.h>
#ifndef LIBHX_DBG_INLINE
#	define LIBHX_DBG_INLINE static inline
#endif
#if !defined(LIBHX_OPT_O2) && defined(__GNUC__) && !defined(__clang__) && !defined(__llvm__)
#	define LIBHX_OPT_O2 __attribute__((optimize("-O2")))
#else
#	define LIBHX_OPT_O2
#endif

LIBHX_DBG_INLINE uint16_t LIBHX_OPT_O2 be16p_to_cpu(const void *p)
{
	const uint8_t *q = (const uint8_t *)p;
	typedef uint16_t T;
	return (T)q[1] | ((T)q[0] << 8);
}

LIBHX_DBG_INLINE uint16_t LIBHX_OPT_O2 le16p_to_cpu(const void *p)
{
	const uint8_t *q = (const uint8_t *)p;
	typedef uint16_t T;
	return (T)q[0] | ((T)q[1] << 8);
}

LIBHX_DBG_INLINE uint32_t LIBHX_OPT_O2 be32p_to_cpu(const void *p)
{
	const uint8_t *q = (const uint8_t *)p;
	typedef uint32_t T;
	return (T)q[3] | ((T)q[2] << 8) | ((T)q[1] << 16) | ((T)q[0] << 24);
}

LIBHX_DBG_INLINE uint32_t LIBHX_OPT_O2 le32p_to_cpu(const void *p)
{
	const uint8_t *q = (const uint8_t *)p;
	typedef uint32_t T;
	return (T)q[0] | ((T)q[1] << 8) | ((T)q[2] << 16) | ((T)q[3] << 24);
}

LIBHX_DBG_INLINE uint64_t LIBHX_OPT_O2 be64p_to_cpu(const void *p)
{
	const uint8_t *q = (const uint8_t *)p;
	typedef uint64_t T;
	return (T)q[7] | ((T)q[6] << 8) | ((T)q[5] << 16) | ((T)q[4] << 24) | ((T)q[3] << 32) |
	       ((T)q[2] << 40) | ((T)q[1] << 48) | ((T)q[0] << 56);
}

LIBHX_DBG_INLINE uint64_t LIBHX_OPT_O2 le64p_to_cpu(const void *p)
{
	const uint8_t *q = (const uint8_t *)p;
	typedef uint64_t T;
	return (T)q[0] | ((T)q[1] << 8) | ((T)q[2] << 16) | ((T)q[3] << 24) | ((T)q[4] << 32) |
	       ((T)q[5] << 40) | ((T)q[6] << 48) | ((T)q[7] << 56);
}

LIBHX_DBG_INLINE void LIBHX_OPT_O2 cpu_to_be16p(void *p, uint16_t v)
{
	uint8_t *q = (uint8_t *)p;
	q[1] = v;
	q[0] = v >> 8;
}

LIBHX_DBG_INLINE void LIBHX_OPT_O2 cpu_to_le16p(void *p, uint16_t v)
{
	uint8_t *q = (uint8_t *)p;
	q[0] = v;
	q[1] = v >> 8;
}

LIBHX_DBG_INLINE void LIBHX_OPT_O2 cpu_to_be32p(void *p, uint32_t v)
{
	uint8_t *q = (uint8_t *)p;
	q[3] = v;
	q[2] = v >> 8;
	q[1] = v >> 16;
	q[0] = v >> 24;
}

LIBHX_DBG_INLINE void LIBHX_OPT_O2 cpu_to_le32p(void *p, uint32_t v)
{
	uint8_t *q = (uint8_t *)p;
	q[0] = v;
	q[1] = v >> 8;
	q[2] = v >> 16;
	q[3] = v >> 24;
}

LIBHX_DBG_INLINE void LIBHX_OPT_O2 cpu_to_be64p(void *p, uint64_t v)
{
	uint8_t *q = (uint8_t *)p;
	q[7] = v;
	q[6] = v >> 8;
	q[5] = v >> 16;
	q[4] = v >> 24;
	q[3] = v >> 32;
	q[2] = v >> 40;
	q[1] = v >> 48;
	q[0] = v >> 56;
}

LIBHX_DBG_INLINE void LIBHX_OPT_O2 cpu_to_le64p(void *p, uint64_t v)
{
	uint8_t *q = (uint8_t *)p;
	q[0] = v;
	q[1] = v >> 8;
	q[2] = v >> 16;
	q[3] = v >> 24;
	q[4] = v >> 32;
	q[5] = v >> 40;
	q[6] = v >> 48;
	q[7] = v >> 56;
}

/* This is in essence the same as htons/htonl/htonll/htonq */
/* [Do we need to worry about trap representations?] */
LIBHX_DBG_INLINE uint16_t LIBHX_OPT_O2 be16_to_cpu(uint16_t v)
{
	return be16p_to_cpu(&v);
}

LIBHX_DBG_INLINE uint32_t LIBHX_OPT_O2 be32_to_cpu(uint32_t v)
{
	return be32p_to_cpu(&v);
}

LIBHX_DBG_INLINE uint64_t LIBHX_OPT_O2 be64_to_cpu(uint64_t v)
{
	return be64p_to_cpu(&v);
}

LIBHX_DBG_INLINE uint16_t LIBHX_OPT_O2 le16_to_cpu(uint16_t v)
{
	return le16p_to_cpu(&v);
}

LIBHX_DBG_INLINE uint32_t LIBHX_OPT_O2 le32_to_cpu(uint32_t v)
{
	return le32p_to_cpu(&v);
}

LIBHX_DBG_INLINE uint64_t LIBHX_OPT_O2 le64_to_cpu(uint64_t v)
{
	return le64p_to_cpu(&v);
}

LIBHX_DBG_INLINE uint16_t LIBHX_OPT_O2 cpu_to_be16(uint16_t v)
{
	cpu_to_be16p(&v, v);
	return v;
}

LIBHX_DBG_INLINE uint32_t LIBHX_OPT_O2 cpu_to_be32(uint32_t v)
{
	cpu_to_be32p(&v, v);
	return v;
}

LIBHX_DBG_INLINE uint64_t LIBHX_OPT_O2 cpu_to_be64(uint64_t v)
{
	cpu_to_be64p(&v, v);
	return v;
}

LIBHX_DBG_INLINE uint16_t LIBHX_OPT_O2 cpu_to_le16(uint16_t v)
{
	cpu_to_le16p(&v, v);
	return v;
}

LIBHX_DBG_INLINE uint32_t LIBHX_OPT_O2 cpu_to_le32(uint32_t v)
{
	cpu_to_le32p(&v, v);
	return v;
}

LIBHX_DBG_INLINE uint64_t LIBHX_OPT_O2 cpu_to_le64(uint64_t v)
{
	cpu_to_le64p(&v, v);
	return v;
}

#endif /* _LIBHX_ENDIAN_H */
