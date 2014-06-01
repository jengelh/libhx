/*
 *	String placeholder expansion
 *	Copyright Jan Engelhardt, 2007-2010
 *
 *	This file is part of libHX. libHX is free software; you can
 *	redistribute it and/or modify it under the terms of the GNU Lesser
 *	General Public License as published by the Free Software Foundation;
 *	either version 2.1 or (at your option) any later version.
 */
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libHX.h>
#include "internal.h"

/* To make it easier on the highlighter */
#define C_OPEN  '('
#define C_CLOSE ')'
#define S_OPEN  "("
#define S_CLOSE ")"

/**
 * %HXFMT_ARGSEP_NONE:	function takes only a single argument
 * %HXFMT_ARGSEP_SPACE: split arguments at whitespace
 * 			e.g. %(exec /bin/ls foo)
 * %HXFMT_ARGSEP_COMMA:	split arguments at comma
 * 			e.g. %(if %(this),%(then),%(else))
 */
enum {
	HXFMT_ARGSEP_NONE  = 0,
	HXFMT_ARGSEP_SPACE = 1 << 0,
	HXFMT_ARGSEP_COMMA = 1 << 1,
};

struct fmt_entry {
	const void *ptr;
	unsigned int type;
};

struct func_entry {
	hxmc_t *(*proc)(int, const hxmc_t *const *, const struct HXformat_map *);
	char delim[4];
};

struct HXformat2_fd {
	const char *name;
	hxmc_t *(*proc)(int, const hxmc_t *const *, const struct HXformat_map *);
	unsigned int flags;
};

struct HXformat_map {
	struct HXmap *vars;
	struct HXmap *funcs;
};

static void fmt_entry_free(void *e)
{
	struct fmt_entry *entry = e;

	switch (entry->type) {
	case HXTYPE_STRING | HXFORMAT_IMMED:
		free(const_cast1(void *, entry->ptr));
		break;
	case HXTYPE_MCSTR | HXFORMAT_IMMED:
		HXmc_free(const_cast1(void *, entry->ptr));
		break;
	}
	free(entry);
}

static const struct HXmap_ops fmt_entry_ops = {
	.d_free = fmt_entry_free,
};

static void *func_entry_clone(const void *data, size_t size)
{
	const struct HXformat2_fd *in = data;
	struct func_entry *out;
	unsigned int i = 0;

	out = malloc(sizeof(*out));
	if (out == NULL)
		return NULL;
	out->proc = in->proc;
	memset(out->delim, '\0', sizeof(out->delim));
	out->delim[i++] = C_CLOSE;
	if (in->flags & HXFMT_ARGSEP_COMMA)
		out->delim[i++] = ',';
	if (in->flags & HXFMT_ARGSEP_SPACE)
		out->delim[i++] = ' ';
	return out;
}

static const struct HXmap_ops func_entry_ops = {
	.d_clone = func_entry_clone,
};

EXPORT_SYMBOL void HXformat_free(struct HXformat_map *blk)
{
	HXmap_free(blk->vars);
	HXmap_free(blk->funcs);
	free(blk);
}

EXPORT_SYMBOL int HXformat_add(struct HXformat_map *blk, const char *key,
    const void *ptr, unsigned int ptr_type)
{
	struct fmt_entry *entry;
	int ret;

	if (strpbrk(key, "\t\n\v ") != NULL || *key == '\0') {
		fprintf(stderr, "%s: Bogus key \"%s\"\n", __func__, key);
		return -EINVAL;
	}

	if ((entry = malloc(sizeof(*entry))) == NULL)
		return -errno;

	entry->type = ptr_type;
	if (ptr_type == (HXTYPE_STRING | HXFORMAT_IMMED)) {
		if ((entry->ptr = HX_strdup(ptr)) == NULL) {
			free(entry);
			return -errno;
		}
	} else if (ptr_type == (HXTYPE_MCSTR | HXFORMAT_IMMED)) {
		if ((entry->ptr = HXmc_meminit(ptr, HXmc_length(ptr))) == NULL) {
			free(entry);
			return -errno;
		}
	} else {
		entry->ptr = ptr;
	}

	ret = HXmap_add(blk->vars, key, entry);
	if (ret <= 0) {
		free(entry);
		return ret;
	}

	return 1;
}

static __inline__ char *HX_strchr0(const char *s, char c)
{
	char *ret = strchr(s, c);
	if (ret != NULL)
		return ret;
	return const_cast1(char *, s) + strlen(s);
}

/*
 * Used as an unique object for "expanded to nothing", to distinguish it from
 * %NULL indicating some error.
 */
static char HXformat2_nexp;

static __inline__ void HXformat2_insuf(const char *func, int argc)
{
	fprintf(stderr, "%s: insufficient number of arguments (%d)\n",
	        func, argc);
}

/*
 * Echo input back, with markers. This is strictly for testing only.
 */
static hxmc_t *HXformat2_echo(int argc, const hxmc_t *const *argv,
                              const struct HXformat_map *blk)
{
	hxmc_t *ret = HXmc_meminit(NULL, 0);
	int i;

	HXmc_strcat(&ret, "<echo");
	for (i = 0; i < argc; ++i) {
		HXmc_strcat(&ret, " [");
		HXmc_strcat(&ret, argv[i]);
		HXmc_strcat(&ret, "]");
	}
	HXmc_strcat(&ret, ">");
	return ret;
}

static hxmc_t *HXformat2_env(int argc, const hxmc_t *const *argv,
                             const struct HXformat_map *blk)
{
	const char *s;

	if (argc == 0)
		return &HXformat2_nexp;
	s = getenv(argv[0]);
	return (s == NULL) ? &HXformat2_nexp : HXmc_strinit(s);
}

static hxmc_t *HXformat2_if(int argc, const hxmc_t *const *argv,
                            const struct HXformat_map *blk)
{
	if (argc < 2) {
		HXformat2_insuf(__func__, argc);
		return &HXformat2_nexp;
	}

	if (*argv[0] != '\0')
		return (*argv[1] != '\0') ?
		       HXmc_strinit(argv[1]) : &HXformat2_nexp;

	return (argc >= 3 && *argv[2] != '\0') ?
	       HXmc_strinit(argv[2]) : &HXformat2_nexp;
}

static hxmc_t *HXformat2_lower(int argc, const hxmc_t *const *argv,
                               const struct HXformat_map *blk)
{
	hxmc_t *ret;

	if (argc == 0)
		return &HXformat2_nexp;
	ret = HXmc_strinit(argv[0]);
	HX_strlower(ret);
	return ret;
}

static hxmc_t *HXformat2_exec1(const hxmc_t *const *argv,
			       const struct HXformat_map *blk)
{
	struct HXproc proc = {
		.p_flags = HXPROC_NULL_STDIN | HXPROC_STDOUT | HXPROC_VERBOSE,
	};
	hxmc_t *slurp, *complete = NULL;
	ssize_t ret;

	if (HXmap_find(blk->vars, "/libhx/exec") == NULL)
		return &HXformat2_nexp;

	slurp = HXmc_meminit(NULL, BUFSIZ);
	if (slurp == NULL)
		return NULL;
	complete = HXmc_meminit(NULL, BUFSIZ);
	if (complete == NULL)
		goto out;

	ret = HXproc_run_async(argv, &proc);
	if (ret < 0)
		goto out;
	while ((ret = read(proc.p_stdout, slurp, BUFSIZ)) > 0)
		if (HXmc_memcat(&complete, slurp, ret) == NULL)
			break;
	close(proc.p_stdout);
	HXproc_wait(&proc);
	HXmc_free(slurp);
	return complete;
 out:
	HXmc_free(complete);
	HXmc_free(slurp);
	return &HXformat2_nexp;
}

static hxmc_t *HXformat2_exec(int argc, const hxmc_t *const *argv,
                              const struct HXformat_map *blk)
{
	if (argc == 0)
		return &HXformat2_nexp;
	return HXformat2_exec1(argv, blk);
}

static hxmc_t *HXformat2_shell(int argc, const hxmc_t *const *argv,
                               const struct HXformat_map *blk)
{
	const char *cmd[] = {"/bin/sh", "-c", NULL, NULL};
	if (argc == 0)
		return &HXformat2_nexp;
	cmd[2] = argv[0];
	return HXformat2_exec1(cmd, blk);
}

static hxmc_t *HXformat2_snl(int argc, const hxmc_t *const *argv,
                             const struct HXformat_map *blk)
{
	hxmc_t *s;
	char *p;

	if (argc == 0)
		return &HXformat2_nexp;
	p = s = HXmc_strinit(*argv);
	if (s == NULL)
		return NULL;
	HX_chomp(s);
	while ((p = strchr(p, '\n')) != NULL)
		*p++ = ' ';
	return s;
}

static hxmc_t *HXformat2_substr(int argc, const hxmc_t *const *argv,
                                const struct HXformat_map *blk)
{
	ssize_t offset, length, z;
	hxmc_t *ret;
	char *end;

	if (argc < 2) {
		HXformat2_insuf(__func__, argc);
		return &HXformat2_nexp;
	}

	offset = strtoll(argv[1], &end, 0);
	if (*end != '\0') {
		fprintf(stderr, "HXformat2-substr: found garbage in "
		        "offset specification\n");
		return &HXformat2_nexp;
	}

	z = strlen(argv[0]);
	if (offset < 0)
		offset = z + offset;
	if (offset >= z)
		return &HXformat2_nexp;

	if (argc == 2) {
		if (offset < 0)
			offset = 0;
		length = z - offset;
	} else {
		length = strtoll(argv[2], &end, 0);
		if (*end != '\0') {
			fprintf(stderr, "HXformat2-substr; found garbage in "
			        "length specification\n");
			return &HXformat2_nexp;
		}
		if (length < 0)
			length/*end*/ = z + length;
		else
			length/*end*/ = offset + length;
		if (offset < 0)
			offset = 0;
	}
	if (length <= 0)
		return &HXformat2_nexp;

	ret = HXmc_meminit(NULL, length);
	if (ret == NULL)
		return &HXformat2_nexp;
	if (HXmc_memcpy(&ret, &argv[0][offset], length) == NULL) {
		HXmc_free(ret);
		return &HXformat2_nexp;
	}
	return ret;
}

static hxmc_t *HXformat2_upper(int argc, const hxmc_t *const *argv,
                               const struct HXformat_map *blk)
{
	hxmc_t *ret;

	if (argc == 0)
		return &HXformat2_nexp;
	ret = HXmc_strinit(argv[0]);
	HX_strupper(ret);
	return ret;
}

static const struct HXformat2_fd HXformat2_fmap[] = {
	{"echo",   HXformat2_echo,   HXFMT_ARGSEP_COMMA | HXFMT_ARGSEP_SPACE},
	{"env",    HXformat2_env,    HXFMT_ARGSEP_COMMA | HXFMT_ARGSEP_SPACE},
	{"exec",   HXformat2_exec,   HXFMT_ARGSEP_SPACE},
	{"if",     HXformat2_if,     HXFMT_ARGSEP_COMMA},
	{"lower",  HXformat2_lower,  HXFMT_ARGSEP_NONE},
	{"shell",  HXformat2_shell,  HXFMT_ARGSEP_NONE},
	{"snl",    HXformat2_snl,    HXFMT_ARGSEP_NONE},
	{"substr", HXformat2_substr, HXFMT_ARGSEP_COMMA},
	{"upper",  HXformat2_upper,  HXFMT_ARGSEP_NONE},
};

/**
 * HXformat2_xcall - expand function call (gather args)
 * @name:	name of function
 * @pptr:	pointer to position in string
 * @table:	table of known variables
 *
 * @*pptr must point to the first character of the first argument to the
 * function.
 */
static hxmc_t *HXformat2_xcall(const char *name, const char **pptr,
    const struct HXformat_map *blk)
{
	const struct func_entry *entry;
	hxmc_t *ret, *ret2, **argv;
	struct HXdeque *dq;
	const char *s, *delim;
	int err = 0;

	dq = HXdeque_init();
	if (dq == NULL)
		return NULL;

	entry = HXmap_get(blk->funcs, name);
	delim = (entry != NULL) ? entry->delim : S_CLOSE;
	if (**pptr == C_CLOSE)
		++*pptr;
	else for (s = *pptr; *s != '\0'; s = ++*pptr) {
		while (HX_isspace(*s))
			++s;
		*pptr = s;
		ret = HXparse_dequote_fmt(s, delim, pptr);
		if (ret == NULL)
			goto out_h_errno;
		if (strstr(ret, "%" S_OPEN) != NULL) {
			ret2 = NULL;
			err = HXformat_aprintf(blk, &ret2, ret);
			if (err < 0 || ret2 == NULL)
				goto out_h_neg;
			HXmc_free(ret);
			ret = ret2;
		}
		if (HXdeque_push(dq, ret) == NULL)
			goto out_h_errno;
		if (**pptr == '\0')
			break;
		if (**pptr == C_CLOSE) {
			++*pptr;
			break;
		}
	}

	ret = NULL;
	argv = reinterpret_cast(hxmc_t **, HXdeque_to_vec(dq, NULL));
	if (argv == NULL)
		goto out_h_errno;

	ret = &HXformat2_nexp;
	/* Unknown functions are silently expanded to nothing, like in make. */
	if (entry != NULL)
		ret = entry->proc(dq->items,
		      const_cast2(const hxmc_t *const *, argv),
		      blk);
	/*
	 * Pointers in argv are shared with those in dq.
	 * Free only the outer shell of one.
	 */
	free(argv);
 out:
	HXdeque_genocide2(dq, static_cast(void *, HXmc_free));
	errno = -err;
	return ret;

 out_h_errno:
	err = -errno;
 out_h_neg:
	HXmc_free(ret);
	ret = NULL;
	goto out;
}

/**
 * HXformat2_xvar - expand a variable
 * @entry:	the variable to expand
 */
static hxmc_t *HXformat2_xvar(const struct fmt_entry *entry)
{
#define IMM(fmt, type) \
	snprintf(buf, sizeof(buf), (fmt), \
		static_cast(type, reinterpret_cast(uintptr_t, entry->ptr))); \
	break;
#define PTR(fmt, type) \
	snprintf(buf, sizeof(buf), (fmt), \
		*static_cast(const type *, entry->ptr)); \
	break;

	static const char *const tf[] = {"false", "true"};
	char buf[HXSIZEOF_Z64];
	hxmc_t *wp = NULL;

	*buf = '\0';
	switch (entry->type) {
		case HXTYPE_STRING:
		case HXTYPE_STRING | HXFORMAT_IMMED:
			HXmc_strcpy(&wp, entry->ptr);
			break;
		case HXTYPE_STRP:
			HXmc_strcpy(&wp, *static_cast(const char *const *, entry->ptr));
			break;
		case HXTYPE_MCSTR:
		case HXTYPE_MCSTR | HXFORMAT_IMMED: {
			const hxmc_t *input = entry->ptr;
			HXmc_memcpy(&wp, input, HXmc_length(input));
			break;
		}
		case HXTYPE_BOOL:
			HXmc_strcpy(&wp, tf[!!*static_cast(const int *,
			           entry->ptr)]);
			break;
		case HXTYPE_BOOL | HXFORMAT_IMMED:
			HXmc_strcpy(&wp, tf[entry->ptr != NULL]);
			break;

		case HXTYPE_BYTE:   PTR("%c", unsigned char);
		case HXTYPE_SHORT:  PTR("%hd", short);
		case HXTYPE_USHORT: PTR("%hu", unsigned short);
		case HXTYPE_CHAR:   PTR("%d", char);
		case HXTYPE_UCHAR:  PTR("%u", unsigned char);
		case HXTYPE_INT:    PTR("%d", int);
		case HXTYPE_UINT:   PTR("%u", unsigned int);
		case HXTYPE_LONG:   PTR("%ld", long);
		case HXTYPE_ULONG:  PTR("%lu", unsigned long);
		case HXTYPE_LLONG:  PTR("%" HX_LONGLONG_FMT "d", long long);
		case HXTYPE_ULLONG: PTR("%" HX_LONGLONG_FMT "u", unsigned long long);
		case HXTYPE_FLOAT:  PTR("%f", float);
		case HXTYPE_DOUBLE: PTR("%f", double);

		case HXTYPE_CHAR   | HXFORMAT_IMMED: IMM("%d", char);
		case HXTYPE_UCHAR  | HXFORMAT_IMMED: IMM("%u", unsigned char);
		case HXTYPE_SHORT  | HXFORMAT_IMMED: IMM("%hd", short);
		case HXTYPE_USHORT | HXFORMAT_IMMED: IMM("%hu", unsigned short);
		case HXTYPE_INT    | HXFORMAT_IMMED: IMM("%d", int);
		case HXTYPE_UINT   | HXFORMAT_IMMED: IMM("%u", unsigned int);
		case HXTYPE_LONG   | HXFORMAT_IMMED: IMM("%ld", long);
		case HXTYPE_ULONG  | HXFORMAT_IMMED: IMM("%lu", unsigned long);

		default:
			fprintf(stderr, "%s: Illegal type\n", __func__);
			return &HXformat2_nexp;
	}

	if (*buf != '\0')
		HXmc_strcpy(&wp, buf);
	return wp;
#undef IMM
#undef PTR
}

/**
 * HXformat2_xany - expand function call or variable
 * @pptr:	pointer to position in string
 * @table:	map of available variables
 *
 * @*pptr has to point to the first character after the "%(" opener.
 */
static hxmc_t *
HXformat2_xany(const char **pptr, const struct HXformat_map *blk)
{
	const char *s = *pptr;
	hxmc_t *name, *ret;

	/*
	 * Some shortcuts for cases that always expand to nothing.
	 * %() and %( ).
	 */
	if (*s == C_CLOSE) {
		++*pptr;
		return &HXformat2_nexp;
	} else if (HX_isspace(*s)) {
		while (HX_isspace(*s))
			++s;
		HXmc_free(HXparse_dequote_fmt(s, S_CLOSE, pptr));
		++*pptr;
		return &HXformat2_nexp;
	}

	/* Long parsing */
	name = HXparse_dequote_fmt(s, S_CLOSE " \t\n\f\v\r", pptr);
	if (name == NULL)
		return NULL;
	s = *pptr;
	if (*s == '\0') {
		fprintf(stderr, "libHX-format2: "
		        "unterminated variable reference / "
		        "missing closing parenthesis.\n");
		return NULL;
	} else if (*s == C_CLOSE) {
		/* Closing parenthesis - variable */
		const struct fmt_entry *entry;
		hxmc_t *new_name = NULL;
		int eret;

		*pptr = ++s;
		eret  = HXformat_aprintf(blk, &new_name, name);
		if (eret <= 0) {
			ret = NULL;
		} else if (*new_name == '\0') {
			ret = &HXformat2_nexp;
		} else {
			entry = HXmap_get(blk->vars, new_name);
			ret   = (entry == NULL) ? &HXformat2_nexp :
			        HXformat2_xvar(entry);
		}
		HXmc_free(new_name);
	} else {
		/* Seen whitespace - function */
		while (HX_isspace(*s))
			++s;
		*pptr = s;
		/*
		 * Note that %() is not expanded in function names. This
		 * follows make(1) behavior.
		 */
		ret = HXformat2_xcall(name, pptr, blk);
	}

	HXmc_free(name);
	return ret;
}

EXPORT_SYMBOL struct HXformat_map *HXformat_init(void)
{
	struct HXformat_map *blk;
	unsigned int i;
	int saved_errno, ret;

	blk = calloc(1, sizeof(*blk));
	if (blk == NULL)
		return NULL;

	blk->vars = HXmap_init5(HXMAPT_DEFAULT, HXMAP_SCKEY, &fmt_entry_ops,
	            0, sizeof(struct fmt_entry));
	if (blk->vars == NULL)
		goto out;
	blk->funcs = HXmap_init5(HXMAPT_DEFAULT, HXMAP_SCKEY, &func_entry_ops,
	             0, sizeof(struct func_entry));
	if (blk->funcs == NULL)
		goto out;
	for (i = 0; i < ARRAY_SIZE(HXformat2_fmap); ++i) {
		ret = HXmap_add(blk->funcs, HXformat2_fmap[i].name,
		      &HXformat2_fmap[i]);
		if (ret < 0)
			goto out;
	}
	return blk;

 out:
	saved_errno = errno;
	if (blk->vars != NULL)
		HXmap_free(blk->vars);
	if (blk->funcs != NULL)
		HXmap_free(blk->funcs);
	free(blk);
	errno = saved_errno;
	return NULL;
}

EXPORT_SYMBOL int HXformat_aprintf(const struct HXformat_map *blk,
    hxmc_t **resultp, const char *fmt)
{
	hxmc_t *ex, *ts, *out;
	const char *current;
	int ret = 0;

	out = HXmc_strinit("");
	if (out == NULL)
		goto out;

	current = fmt;
	while ((current = HX_strchr0(fmt, '%')) != NULL) {
		if (current - fmt > 0)
			if (HXmc_memcat(&out, fmt, current - fmt) == NULL)
				goto out;
		if (*current == '\0')
			break;
		if (current[1] != C_OPEN) {
			if (HXmc_memcat(&out, current, 2) == NULL)
				goto out;
			fmt = current + 2;
			continue;
		}

		current += 2; /* skip % and opening parenthesis */
		ex = HXformat2_xany(&current, blk);
		if (ex == NULL)
			goto out;
		if (ex != &HXformat2_nexp) {
			ts = HXmc_memcat(&out, ex, HXmc_length(ex));
			HXmc_free(ex);
			if (ts == NULL)
				goto out;
		}
		fmt = current;
	}

	*resultp = out;
	return HXmc_length(out);

 out:
	ret = -errno;
	HXmc_free(out);
	return ret;
}

EXPORT_SYMBOL int HXformat_fprintf(const struct HXformat_map *ftable,
    FILE *filp, const char *fmt)
{
	hxmc_t *str;
	int ret;

	if ((ret = HXformat_aprintf(ftable, &str, fmt)) <= 0)
		return ret;
	errno = 0;
	if (fputs(str, filp) < 0)
		ret = -errno;
	HXmc_free(str);
	return ret;
}

EXPORT_SYMBOL int HXformat_sprintf(const struct HXformat_map *ftable,
    char *dest, size_t size, const char *fmt)
{
	hxmc_t *str;
	int ret;

	if ((ret = HXformat_aprintf(ftable, &str, fmt)) < 0)
		return ret;
	if (ret == 0) {
		*dest = '\0';
		return 0;
	}
	strncpy(dest, str, size);
	ret = HXmc_length(dest);
	HXmc_free(str);
	return ret;
}
