#ifndef _LIBHX_ENDIAN_FLOAT_H
#define _LIBHX_ENDIAN_FLOAT_H 1
#include <string.h>
#include <libHX/endian.h>

/*
 * While construction of integers from bytes was easy, it would be more work
 * for floats — and compilers probably won't be able to optimize it.
 *
 * So then, we make some shortcuts/assumptions here in endian_float.h:
 * - that the host platform uses the same byte order for integers as for floats
 * - that the host platform is using IEEE754/IEC559
 *
 * This holds for the typical Linux on {arm gnueabi LE, arm gnueabi
 * BE, aarch64 LE, aarch64 BE, i386, amd64, hppa, loongarch64, m68k,
 * mips, ppc64, ppc64le, sparc, sparc64, riscv64, s390x}.
 */

/*
 * Unlike cpu_to_be32, we will offer no float_cpu_to_be32. Values comprised of
 * inverted bytes should probably not be passed around in memory.
 */
LIBHX_DBG_INLINE float LIBHX_OPT_O2 float_be32p_to_cpu(const void *p)
{
	uint32_t v = be32p_to_cpu(p);
	float w;
#ifdef __cplusplus
	static_assert(sizeof(v) == sizeof(w));
#endif
	memcpy(&w, &v, sizeof(w));
	return w;
}

LIBHX_DBG_INLINE double LIBHX_OPT_O2 float_le32p_to_cpu(const void *p)
{
	uint32_t v = le32p_to_cpu(p);
	float w;
#ifdef __cplusplus
	static_assert(sizeof(v) == sizeof(w));
#endif
	memcpy(&w, &v, sizeof(w));
	return w;
}

LIBHX_DBG_INLINE float LIBHX_OPT_O2 float_be64p_to_cpu(const void *p)
{
	uint64_t v = be64p_to_cpu(p);
	double w;
#ifdef __cplusplus
	static_assert(sizeof(v) == sizeof(w));
#endif
	memcpy(&w, &v, sizeof(w));
	return w;
}

LIBHX_DBG_INLINE double LIBHX_OPT_O2 float_le64p_to_cpu(const void *p)
{
	uint64_t v = le64p_to_cpu(p);
	double w;
#ifdef __cplusplus
	static_assert(sizeof(v) == sizeof(w));
#endif
	memcpy(&w, &v, sizeof(w));
	return w;
}

LIBHX_DBG_INLINE void LIBHX_OPT_O2 float_cpu_to_be32p(void *p, float v)
{
	uint32_t w;
#ifdef __cplusplus
	static_assert(sizeof(v) == sizeof(w));
#endif
	memcpy(&w, &v, sizeof(w));
	cpu_to_be32p(p, w);
}

LIBHX_DBG_INLINE void LIBHX_OPT_O2 float_cpu_to_le32p(void *p, float v)
{
	uint32_t w;
#ifdef __cplusplus
	static_assert(sizeof(v) == sizeof(w));
#endif
	memcpy(&w, &v, sizeof(w));
	cpu_to_le32p(p, w);
}

LIBHX_DBG_INLINE void LIBHX_OPT_O2 float_cpu_to_be64p(void *p, double v)
{
	uint64_t w;
#ifdef __cplusplus
	static_assert(sizeof(v) == sizeof(w));
#endif
	memcpy(&w, &v, sizeof(w));
	cpu_to_be64p(p, w);
}

LIBHX_DBG_INLINE void LIBHX_OPT_O2 float_cpu_to_le64p(void *p, double v)
{
	uint64_t w;
#ifdef __cplusplus
	static_assert(sizeof(v) == sizeof(w));
#endif
	memcpy(&w, &v, sizeof(w));
	cpu_to_le64p(p, w);
}

#endif /* _LIBHX_ENDIAN_FLOAT_H */
