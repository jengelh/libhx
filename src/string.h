#ifndef _LIBHX_STRING_H
#define _LIBHX_STRING_H 1

#include <sys/types.h>
#ifdef __cplusplus
#	include <cstdio>
#	include <cstdlib>
#	include <cstring>
#else
#	include <stdio.h>
#	include <stdlib.h>
#	include <string.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __libhx_internal_hmc_t_defined
#define __libhx_internal_hmc_t_defined 1
typedef char hmc_t;
#endif

/*
 *	HMC.C
 */
extern hmc_t *hmc_dup(const void *);
extern hmc_t *hmc_sinit(const char *);
extern hmc_t *hmc_minit(const void *, size_t);
extern hmc_t *hmc_strasg(hmc_t **, const char *);
extern hmc_t *hmc_memasg(hmc_t **, const void *, size_t);
extern size_t hmc_length(hmc_t *);
extern hmc_t *hmc_trunc(hmc_t **, size_t);
extern hmc_t *hmc_strcat(hmc_t **, const char *);
extern hmc_t *hmc_memcat(hmc_t **, const void *, size_t);
extern hmc_t *hmc_strpcat(hmc_t **, const char *);
extern hmc_t *hmc_mempcat(hmc_t **, const void *, size_t);
extern hmc_t *hmc_strins(hmc_t **, size_t, const char *);
extern hmc_t *hmc_memins(hmc_t **, size_t, const void *, size_t);
extern hmc_t *hmc_memdel(hmc_t *, size_t, size_t);
extern void hmc_free(hmc_t *);

/*
 *	STRING.C
 */
extern char *HX_basename(const char *);
extern char *HX_chomp(char *);
extern char *HX_dirname(const char *);
extern hmc_t *HX_getl(hmc_t **, FILE *);
extern char **HX_split(const char *, const char *, int *, int);
extern int HX_split5(char *, const char *, int, char **);
extern char *HX_strbchr(const char *, const char *, char);
extern char *HX_strclone(char **, const char *);
extern char *HX_strlower(char *);
extern size_t HX_strltrim(char *);
extern char *HX_strmid(const char *, long, long);
extern size_t HX_strrcspn(const char *, const char *);
extern char *HX_strrev(char *);
extern size_t HX_strrtrim(char *);
extern char *HX_strsep(char **, const char *);
extern char *HX_strsep2(char **, const char *);
extern char *HX_strupper(char *);

static inline char *HX_strlcat(char *dest, const char *src, size_t len)
{
	ssize_t x = len - strlen(dest) - 1;
	if (x <= 0)
		return dest;
	return strncat(dest, src, x);
}

static inline char *HX_strlcpy(char *dest, const char *src, size_t n)
{
	strncpy(dest, src, n);
	dest[n-1] = '\0';
	return dest;
}

static inline char *HX_strlncat(char *dest, const char *src, size_t dlen,
    size_t slen)
{
	ssize_t x = dlen - strlen(dest) - 1;
	if (x <= 0)
		return dest;
	x = ((ssize_t)slen < x) ? (ssize_t)slen : x;
	return strncat(dest, src, x);
}

static inline void *HX_memdup(const void *buf, size_t len)
{
	void *ret;
	if ((ret = malloc(len)) == NULL)
		return NULL;
	return memcpy(ret, buf, len);
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#ifdef __cplusplus
extern "C++" {

template<typename type> static inline type
HX_memdup(const void *data, size_t n)
{
	return reinterpret_cast<type>(HX_memdup(data, n));
}

} /* extern "C++" */
#endif

#ifdef __cplusplus
extern "C" {
#endif

static inline char *HX_strdup(const char *src)
{
	if (src == NULL)
		return NULL;
#ifdef __cplusplus
	return HX_memdup<char *>(src, strlen(src) + 1);
#else
	return HX_memdup(src, strlen(src) + 1);
#endif
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _LIBHX_STRING_H */
