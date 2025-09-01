#ifndef _LIBHX_OPTION_H
#define _LIBHX_OPTION_H 1

#ifdef __cplusplus
#	include <cstddef>
#	include <cstdio>
#else
#	include <stddef.h>
#	include <stdio.h>
#endif
#include <libHX/cast.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __libhx_internal_hxmc_t_defined
#define __libhx_internal_hxmc_t_defined 1
typedef char hxmc_t;
#endif

struct HXoption;

/*
 *	FORMAT.C
 */
extern struct HXformat_map *HXformat_init(void);
extern void HXformat_free(struct HXformat_map *);
extern int HXformat_add(struct HXformat_map *, const char *, const void *,
	unsigned int);
#define HXformat_aprintf(a, b, c) HXformat3_aprintf((a), (b), (c))
#define HXformat_fprintf(a, b, c) HXformat3_fprintf((a), (b), (c))
#define HXformat_sprintf(a, b, c, d) HXformat3_sprintf((a), (b), (c), (d))
extern ssize_t HXformat3_aprintf(const struct HXformat_map *, hxmc_t **, const char *);
extern ssize_t HXformat3_fprintf(const struct HXformat_map *, FILE *, const char *);
extern ssize_t HXformat3_sprintf(const struct HXformat_map *, char *, size_t, const char *);

/*
 *	OPT.C
 */

/**
 * Available types for struct HXoption.type.
 * %HXTYPE_NONE:	[-o] (int *) No argument; counts presence.
 * %HXTYPE_VAL:		[-o] (int *) Set to value in .val.
 * %HXTYPE_SVAL:	[-o] (const char *) Set to value in .uptr.
 * %HXTYPE_BOOL:	[fo] (int *) Parse argument as boolean
 * 			     ("yes", "no", "true", "false", 0 or non-zero)
 * %HXTYPE_BYTE:	[fo] (unsigned char *) Take first char of argument
 * %HXTYPE_UCHAR:	[fo] (unsigned char *) An integer.
 * %HXTYPE_CHAR:	[fo] (char *) An integer.
 * %HXTYPE_USHORT:	[fo] (unsigned short *) An integer.
 * %HXTYPE_SHORT:	[fo] (short *) An integer.
 * %HXTYPE_UINT:	[fo] (unsigned int *) An integer.
 * %HXTYPE_INT:		[fo] (int *) An integer.
 * %HXTYPE_ULONG:	[fo] (unsigned long *) An integer.
 * %HXTYPE_LONG:	[fo] (long *) An integer.
 * %HXTYPE_ULLONG:	[fo] (unsigned long long *) An integer.
 * %HXTYPE_LLONG:	[fo] (long long *) An integer.
 * %HXTYPE_FLOAT:	[fo] (float *) Read a floating point number
 * %HXTYPE_DOUBLE:	[fo] (double *) Read a floating point number
 * %HXTYPE_STRING:	[fo] (char **) Any string.
 * %HXTYPE_STRP:	[f-] (const char *const *) A string.
 * %HXTYPE_STRDQ:	[-o] (struct HXdeque *) A string.
 * %HXTYPE_UINT8:	[-o] (uint8_t *) An integer.
 * %HXTYPE_UINT16:	[-o] (uint8_t *) An integer.
 * %HXTYPE_UINT32:	[-o] (uint8_t *) An integer.
 * %HXTYPE_UINT64:	[-o] (uint8_t *) An integer.
 * %HXTYPE_INT8:	[-o] (uint8_t *) An integer.
 * %HXTYPE_INT16:	[-o] (uint8_t *) An integer.
 * %HXTYPE_INT32:	[-o] (uint8_t *) An integer.
 * %HXTYPE_INT64:	[-o] (uint8_t *) An integer.
 * %HXTYPE_MCSTR:	[-o] (hxmc_t *) A string.
 * %HXTYPE_XSNTMARK:	[-o] Internal sentinal marker (used in HXOPT_TABLEEND)
 * %HXTYPE_XHELP:	[-o] Internal helper marker (used in HXOPT_AUTOHELP)
 * %HXTYPE_SIZE_T:	[-o] (size_t *) An integer.
 *
 * Type expected of struct HXoption.ptr is given in ().
 * HX_getopt (o) and HXformat_* (f) support different sets, marked with [].
 */
enum {
	HXTYPE_NONE = 0,
	HXTYPE_VAL,
	HXTYPE_SVAL,
	HXTYPE_BOOL,
	HXTYPE_BYTE,
	HXTYPE_UCHAR, /* 5 */
	HXTYPE_CHAR,
	HXTYPE_USHORT,
	HXTYPE_SHORT,
	HXTYPE_UINT,
	HXTYPE_INT, /* 10 */
	HXTYPE_ULONG,
	HXTYPE_LONG,
	HXTYPE_ULLONG,
	HXTYPE_LLONG,
	HXTYPE_FLOAT, /* 15 */
	HXTYPE_DOUBLE,
	HXTYPE_STRING,
	HXTYPE_STRP, /* (const char **) */
	HXTYPE_STRDQ,
	HXTYPE_UINT8, /* 20 */
	HXTYPE_UINT16,
	HXTYPE_UINT32,
	HXTYPE_UINT64,
	HXTYPE_INT8,
	HXTYPE_INT16, /* 25 */
	HXTYPE_INT32,
	HXTYPE_INT64,
	HXTYPE_MCSTR,
	HXTYPE_XSNTMARK,
	HXTYPE_XHELP, /* 30 */
	HXTYPE_SIZE_T,

/**
 * Extra flags to be OR'ed into struct HXoption.type.
 * %HXOPT_OPTIONAL:	argument to option is optional
 * 			(it's bad taste to use this)
 * %HXOPT_INC:		increase variable pointed to by .ptr.
 * 			(only applies to %HXTYPE_NONE)
 * %HXOPT_DEC:		increase variable pointed to by .ptr.
 * 			(only applies to %HXTYPE_NONE)
 * %HXOPT_NOT:		negate input (*ptr), this is done before OR/AND
 * %HXOPT_OR:		OR *ptr by argument
 * %HXOPT_AND:		AND *ptr by argument
 * %HXOPT_XOR:		XOR *ptr by argument
 */
	HXOPT_OPTIONAL = 1 << 6,
	HXOPT_INC      = 1 << 7,
	HXOPT_DEC      = 1 << 8,
	HXOPT_NOT      = 1 << 9,
	HXOPT_OR       = 1 << 10,
	HXOPT_AND      = 1 << 11,
	HXOPT_XOR      = 1 << 12,
};

/**
 * Flags to HX_getopt.
 * %HXOPT_PTHRU:	pass-through unknown options to new argv (obsolete)
 * %HXOPT_DESTROY_OLD:	destroy old argv after parsing is successful
 * %HXOPT_QUIET:	do not output any warnings to stderr
 * %HXOPT_HELPONERR:	print out help when a parsing error occurs
 * %HXOPT_USAGEONERR:	print out short usage when a parsing error occurs
 * %HXOPT_RQ_ORDER:     Options and non-options must not be mixed (first
 *                      non-option stops parsing) and the environment variable
 *                      POSIXLY_CORRECT is ignored.
 * %HXOPT_KEEP_ARGV:	do not replace argc/argv at all
 * %HXOPT_ANY_ORDER:    Options and non-options may be mixed and the
 *                      environment variable POSIXLY_CORRECT is ignored.
 * %HXOPT_DUP_ARGS:     (HX_getopt6 only) Populate result.dup_argv.
 */
enum {
	HXOPT_PTHRU       = 0x1U,
	HXOPT_DESTROY_OLD = 0x2U,
	HXOPT_QUIET       = 0x4U,
	HXOPT_HELPONERR   = 0x8U,
	HXOPT_USAGEONERR  = 0x10U,
	HXOPT_RQ_ORDER    = 0x20U,
	HXOPT_KEEP_ARGV   = 0x40U,
	HXOPT_ANY_ORDER   = 0x80U,
	HXOPT_DUP_ARGS    = 0x800U,
};

/**
 * (Positive-ranged) return values for HX_getopt.
 * %HXOPT_ERR_SUCCESS:	success
 * %HXOPT_ERR_UNKN:	unknown option was encountered
 * %HXOPT_ERR_VOID:	long option takes no value
 * %HXOPT_ERR_MIS:	option requires a value argument
 * %HXOPT_ERR_AMBIG:	long option abbreviation was ambiguous
 * %HXOPT_ERR_FLAGS:    illegal flag combination (API misuse)
 */
enum {
	HXOPT_ERR_SUCCESS = 0,
	HXOPT_ERR_UNKN,
	HXOPT_ERR_VOID,
	HXOPT_ERR_MIS,
	HXOPT_ERR_AMBIG,
	HXOPT_ERR_FLAGS,
};

/**
 * Extra flags to be OR'ed into HXformat_add()'s 4th arg.
 * %HXFORMAT_IMMED:	do not dereference the 4th arg to get at the value
 */
enum {
	HXFORMAT_IMMED = 1 << 13,
};

/**
 * Flags for HX_shconfig_pv()
 * %SHCONF_ONE:		only read one configuration file
 */
enum {
	SHCONF_ONE = 1 << 0,
};

/**
 * Flags in struct HXoptcb.flags
 * %HXOPTCB_BY_LONG:	cb was called by invocation of @current->ln
 * %HXOPTCB_BY_SHORT:	cb was called by invocation of @current->sh
 */
enum {
	HXOPTCB_BY_LONG  = 1 << 0,
	HXOPTCB_BY_SHORT = 1 << 1,
};

struct HXoptcb {
	const struct HXoption *table, *current;
	const char *data;
	union {
		double data_dbl;
		long data_long;
	};
	unsigned int flags;
};

/**
 * @ln:		long option string (without "--"), or %NULL
 * @sh:		short option character, or '\0'
 * @type:	type of variable pointed to by .ptr
 * @ptr:	pointer to variable to set/update
 * @uptr:	freeform user-supplied pointer;
 * 		in case of %HXTYPE_SVAL, this is the specific value to set
 * @cb:		callback function to invoke, or %NULL
 * @val:	specific value to set if type == HXTYPE_VAL
 * @help:	help string to display
 * @htyp:	type string to show in option's help
 */
struct HXoption {
	const char *ln;
	char sh;
	unsigned int type;
	void *ptr, *uptr;
	void (*cb)(const struct HXoptcb *);
	int val;
	const char *help, *htyp;
};

/**
 * @dup_argc: String count for dup_argv.
 * @dup_argv: Filled with copies of non-option arguments if %HXOPT_DUP_ARGS.
 *            dup_argv[0] will be the program name (useful for feeding to
 *            another HX_getopt6 call).
 */
struct HXopt6_result {
	int dup_argc;
	char **dup_argv;
};

#ifndef LIBHX_ZVECFREE_DECLARATION
#define LIBHX_ZVECFREE_DECLARATION
extern void HX_zvecfree(char **);
#endif
extern int HX_getopt(const struct HXoption *, int *, char ***, unsigned int);
extern int HX_getopt5(const struct HXoption *, char **argv, int *nargc, char ***nargv, unsigned int flags);
extern int HX_getopt6(const struct HXoption *, int, char **argv, struct HXopt6_result *, unsigned int flags);
extern void HX_getopt6_clean(struct HXopt6_result *);
#define HX_getopt(a, b, c, d) HX_getopt((a), (b), const_cast3(char ***, (c)), (d))
extern void HX_getopt_help(const struct HXoptcb *, FILE *);
extern void HX_getopt_help_cb(const struct HXoptcb *);
extern void HX_getopt_usage(const struct HXoptcb *, FILE *);
extern void HX_getopt_usage_cb(const struct HXoptcb *);
extern int HX_shconfig(const char *, const struct HXoption *);
extern struct HXmap *HX_shconfig_map(const char *);
extern int HX_shconfig_pv(const char **, const char *,
	const struct HXoption *, unsigned int);
extern void HX_shconfig_free(const struct HXoption *);

#ifdef __cplusplus
} /* extern "C" */
#endif

#ifndef __cplusplus
#	define HXOPT_AUTOHELP \
		{.ln = "help", .sh = '?', .type = HXTYPE_XHELP, \
		.cb = HX_getopt_help_cb, .help = "Show this help message"}, \
		{.ln = "usage", .type = HXTYPE_NONE, \
		.cb = HX_getopt_usage_cb, \
		.help = "Display brief usage message"}
#	define HXOPT_TABLEEND {.type = HXTYPE_XSNTMARK}
#else
#	define HXOPT_AUTOHELP \
		{"help", '?', HXTYPE_XHELP, NULL, NULL, HX_getopt_help_cb, \
		0, "Show this help message"}, \
		{"usage", 0, HXTYPE_NONE, NULL, NULL, HX_getopt_usage_cb, \
		0, "Display brief usage message"}
#	define HXOPT_TABLEEND {NULL, 0, HXTYPE_XSNTMARK}
#endif

#endif /* _LIBHX_OPTION_H */
