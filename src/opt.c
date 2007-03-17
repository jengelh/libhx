/*
	libHX/opt.c
	Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2002 - 2007

	This file is part of libHX. libHX is free software; you can
	redistribute it and/or modify it under the terms of the GNU
	Lesser General Public License as published by the Free Software
	Foundation; however ONLY version 2 of the License. For details,
	see the file named "LICENSE.LGPL2".
*/

/*
Feature list:
- bundling ("-a -b -c" is equal to "-abc"; "-a 7" is equal to "-a7")

- passthrough (for option parser chaining)
  (put unknown options back into argv / retrieve leftover arguments)

- handling for a single dash "-" which is commonly used
  to refer to stdin/stdout

- option table based (like popt; unlike libc getopt (string-based))

- only one execution command needed
  (like Perl's Getopt::Long; unlike popt/getopt)

- internal support for HXdeques and HXbtrees ;)
  (like Getopt::Long can accept pointers to arrays / hashes;
  great stuff, lets you do cool things like gcc's -D macro=definition
  in one line!)

- long options with --long=value or --long value

- the usual "--" handling

Not supported:
- "+option"
- "-longoption"
- table inclusion (well, via CBs maybe)
*/

#include <sys/types.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libHX.h"

// Definitions
#define CALL_CB if(opt->cb != NULL) opt->cb(cbi);
#define NTYPE_S(con, tpx) NTYPE((con), tpx, strtol)
#define NTYPE_U(con, tpx) NTYPE((con), tpx, strtoul)
#define NTYPE(con, tpx, func) \
    case (con): { \
        tpx *p, v = (func)(cbi->s, NULL, 0); \
        if((p = opt->ptr) != NULL) { \
            if(opt->type & HXOPT_NOT) v = ~v; \
            switch(opt->type & HXOPT_LOPMASK2) { \
                case 0:         *p  = v; break; \
                case HXOPT_OR:  *p |= v; break; \
                case HXOPT_AND: *p &= v; break; \
                case HXOPT_XOR: *p ^= v; break; \
                default: \
                    fprintf(stderr, "libHX-opt: illegal combination of " \
                     "logical op mask\n"); \
                    break; \
            } \
        } \
        cbi->l = v; \
        CALL_CB; \
        break; \
    }

enum {
    E_SUCCESS = 0,
    E_LONG_UNKNOWN,
    E_LONG_TAKESVOID,
    E_LONG_MISSING,
    E_SHORT_UNKNOWN,
    E_SHORT_MISSING,

    S_NORMAL = 0,
    S_SHORT,
    S_TWOLONG, // yes, "two", not "too"
    S_LONG,
    S_TERMINATED,

    W_NONE    = 0,
    W_SPACE   = 1 << 0,
    W_BRACKET = 1 << 1,
    W_ALT     = 1 << 2,
    W_EQUAL   = 1 << 3,
};

// Functions
static inline const struct HXoption *lookup_long(const struct HXoption *, const char *);
static inline const struct HXoption *lookup_short(const struct HXoption *, char);
static void do_assign(struct HXoptcb *);
static void opt_to_text(const struct HXoption *, char *, size_t, unsigned int);
static void print_indent(const char *, size_t, size_t, FILE *);
static inline char *shell_unescape(char *);
static inline unsigned int takes_void(unsigned int);

//-----------------------------------------------------------------------------
EXPORT_SYMBOL int HX_getopt(const struct HXoption *table, int *argc,
  const char ***argv, unsigned int flags)
{
    const char **opt = *argv, *value = NULL, *shstr = NULL;
    struct HXdeque *remaining = HXdeque_init();
    unsigned int state = S_NORMAL;
    int ret = E_SUCCESS;
    struct HXoptcb cbi;
    char *key = NULL;
    long argk;

    memset(&cbi, 0, sizeof(cbi));
    cbi.arg0  = **argv;
    cbi.table = table;

    HXdeque_push(remaining, HX_strdup(*opt++)); // put argv[0] back

    while(1) {
        const char *cur = *opt;

        if(state == S_TWOLONG) {
            if((cbi.current = lookup_long(table, key)) == NULL) {
                if(flags & HXOPT_PTHRU) {
                    HXdeque_push(remaining, HX_strdup(key));
                    ++opt;
                    state = S_NORMAL;
                    continue;
                }
                ret = E_LONG_UNKNOWN;
                break;
            }

            cbi.tln = key;
            cbi.tsh = 0;

            if(takes_void(cbi.current->type)) {
                cbi.s = NULL;
            } else if(cbi.current->type & HXOPT_OPTIONAL) {
                // Rule: take arg if next thing is not-null, not-option
                if(cur == NULL || *cur != '-' ||
                 (cur[0] == '-' && cur[1] == '\0')) {
                    // --file -, --file bla
                    cbi.s = cur;
                    cur   = *opt++;
                } else {
                    // --file --another, --file -- endofoptions
                    cbi.s = NULL;
                }
            } else {
                if(cur == NULL) {
                    ret = E_LONG_MISSING;
                    break;
                }
                cbi.s = cur;
                cur   = *++opt;
            }

            do_assign(&cbi);
            free(key);
            state = S_NORMAL;
            // fallthrough
        }

        if(state == S_LONG) {
            unsigned int got_value = (strchr(cur, '=') != NULL);

            if((cbi.current = lookup_long(table, key)) == NULL) {
                if(flags & HXOPT_PTHRU) {
                    HXdeque_push(remaining, HX_strdup(*opt++));
                    state = S_NORMAL;
                    continue;
                }
                ret = E_LONG_UNKNOWN;
                break;
            }

            if(takes_void(cbi.current->type) && got_value) {
                ret = E_LONG_TAKESVOID;
                break;
            } else if(!takes_void(cbi.current->type) && !got_value) {
                ret = E_LONG_MISSING;
                break;
            }

            cbi.tln = key;
            cbi.tsh = 0;
            cbi.s   = value;
            do_assign(&cbi);

            free(key);
            state = S_NORMAL;
            cur   = *++opt;
            // fallthrough
        }

        if(state == S_SHORT) {
            if(*shstr == '\0') {
                ++opt;
                state = S_NORMAL;
                continue;
            }

            if((cbi.current = lookup_short(table, *shstr)) == NULL) {
                if(flags & HXOPT_PTHRU) {
                    char buf[16];
                    snprintf(buf, sizeof(buf), "-%s", shstr);
                    HXdeque_push(remaining, HX_strdup(buf));
                    ++opt;
                    state = S_NORMAL;
                    continue;
                }
                ret = E_SHORT_UNKNOWN;
                break;
            }

            cbi.tln = NULL;
            cbi.tsh = *shstr;

            if(takes_void(cbi.current->type)) {
                // -A
                cbi.s = NULL;
                do_assign(&cbi);
                ++shstr;
                continue;
            }

            cur = *++opt;
            if(*(shstr + 1) != '\0') {
                // -Avalue
                cbi.s = shstr + 1;
                do_assign(&cbi);
                state = S_NORMAL;
                continue;
            }

            if(cbi.current->type & HXOPT_OPTIONAL) {
                if(cur == NULL || *cur != '-' ||
                 (cur[0] == '-' && cur[1] == '\0')) {
                    // --file -, --file bla
                    cbi.s = cur;
                    cur   = *++opt;
                } else {
                    // --file --another, --file -- endofoptions
                    cbi.s = NULL;
                }
            } else {
                // -A value
                if(cur == NULL) {
                    ret = E_SHORT_MISSING;
                    break;
                }
                cbi.s = cur;
                cur   = *++opt;
            }

            do_assign(&cbi);
            state = S_NORMAL;
            // fallthrough
        }

        if(cur == NULL)
            break;

        if(state == S_TERMINATED) {
            HXdeque_push(remaining, HX_strdup(*opt++));
            continue;
        }

        if(state == S_NORMAL) {
            if(cur[0] == '-' && cur[1] == '\0') {
                // Note to popt developers: A single dash is NOT an option
                HXdeque_push(remaining, HX_strdup(*opt++));
                continue;
            }
            if(cur[0] == '-' && cur[1] == '-' && cur[2] == '\0') { // double dash
                state = S_TERMINATED;
                /* If passthrough is on, "--" must be copied into
                remaining. This is done in the next round. */
                if(!(flags & HXOPT_PTHRU))
                    // Well, if PTHRU is off, skip it.
                    ++opt;
                continue;
            }
            if(cur[0] == '-' && cur[1] == '-') { // long option
                char *p;
                key = HX_strdup(cur + 2);
                if((p = strchr(key, '=')) == NULL) {
                    // Two argument long option: --long arg
                    state = S_TWOLONG;
                    ++opt;
                    continue;
                }
                // Single argument long option: --long=arg
                *p++  = '\0';
                value = p;
                state = S_LONG;
                continue;
            }
            if(cur[0] == '-') {
                // Short option(s) - one or more(!)
                state = S_SHORT;
                shstr = cur + 1;
                continue;
            }
            HXdeque_push(remaining, HX_strdup(*opt++));
            continue;
        }

        fprintf(stderr, "libHX-opt: invalid state: %u\n", state);
        state = S_NORMAL;
    }

    if(ret != 0) {
        switch(ret) {
            case E_LONG_UNKNOWN:
                if(!(flags & HXOPT_QUIET))
                    fprintf(stderr, "Unknown option: --%s\n", key);
                ret = -HXOPT_ERR_UNKN;
                break;
            case E_LONG_TAKESVOID:
                if(!(flags & HXOPT_QUIET))
                    fprintf(stderr, "Option --%s does not take any argument\n", key);
                ret = -HXOPT_ERR_VOID;
                break;
            case E_LONG_MISSING:
                if(!(flags & HXOPT_QUIET))
                    fprintf(stderr, "Option --%s requires an argument\n", key);
                ret = -HXOPT_ERR_MIS;
                break;
            case E_SHORT_UNKNOWN:
                if(!(flags & HXOPT_QUIET))
                    fprintf(stderr, "Unknown option: -%c\n", *shstr);
                ret = -HXOPT_ERR_UNKN;
                break;
            case E_SHORT_MISSING:
                if(!(flags & HXOPT_QUIET))
                    fprintf(stderr, "Option -%c requires an argument\n", *shstr);
                ret = -HXOPT_ERR_MIS;
                break;
        }
        free(key);

        if(flags & HXOPT_HELPONERR)
            HX_getopt_help(&cbi, stderr);
        else if(flags & HXOPT_USAGEONERR)
            HX_getopt_usage(&cbi, stderr);

        HXdeque_genocide(remaining);
        return ret;
    }

    if(flags & HXOPT_DESTROY_OLD)
        /* Only the "true, original" argv is stored on the stack - the argv
        that HX_getopt() produces is on the heap, so the HXOPT_DESTROY_OLD
        flag should be passed when you use passthrough chaining, i.e. all but
        the first call to HX_getopt() should have this set. */
        HX_zvecfree((char **)*argv);

    *argv = static_cast(const char **, HXdeque_to_vec(remaining, &argk));
    *argc = argk;
    HXdeque_free(remaining);
    return 1;
}

#define SCREEN_WIDTH 80 // popt also has it hardcoded
EXPORT_SYMBOL void HX_getopt_help(const struct HXoptcb *cbi, FILE *nfp)
{
    FILE *fp = (nfp == NULL) ? stderr : nfp;
    const struct HXoption *travp;
    char tmp[84] = {'\0'};
    size_t tw = 0, wd = 0;

    HX_getopt_usage(cbi, nfp);

    // Find maximum indent
    travp = cbi->table;
    while(travp->ln != NULL || travp->sh != 0) {
        size_t tl;
        opt_to_text(travp, tmp, sizeof(tmp), W_EQUAL);
        if((tl = strlen(tmp)) > tw)
            tw = tl;
        ++travp;
    }

    // Print table
    travp = cbi->table;
    while(travp->ln != NULL || travp->sh != 0) {
        opt_to_text(travp, tmp, sizeof(tmp), W_NONE);
        wd = fprintf(fp, "  %-*s    ", static_cast(int, tw), tmp);
        if(travp->help == NULL)
            fprintf(fp, "\n");
        else
            print_indent(travp->help, tw + 6, wd, fp);
        ++travp;
    }
    return;
}

EXPORT_SYMBOL void HX_getopt_help_cb(const struct HXoptcb *cbi)
{
    HX_getopt_help(cbi, stdout);
    exit(EXIT_SUCCESS);
    return;
}

EXPORT_SYMBOL void HX_getopt_usage(const struct HXoptcb *cbi, FILE *nfp)
{
    size_t wd = sizeof("Usage:") + strlen(HX_basename(cbi->arg0)), tw = 0;
    FILE *fp = (nfp == NULL) ? stderr : nfp;
    const struct HXoption *travp;
    char tmp[84] = {'\0'};

    fprintf(fp, "Usage: %s", HX_basename(cbi->arg0));

    // Short-only flags
    if(wd + 5 > SCREEN_WIDTH) {
        // 5 is the minimum size for a new starting option, " [-X]"
        fprintf(fp, "\n     ");
        wd = 6;
    }
    travp = cbi->table;
    while(travp->ln != NULL || travp->sh != 0) {
        if(!(travp->ln == NULL && travp->sh != 0 && takes_void(travp->type))) {
            ++travp;
            continue;
        }
        if(*tmp == '\0') {
            snprintf(tmp, sizeof(tmp), " [-"); // ]
            tw = 3;
        }
        tmp[tw++] = travp->sh;
        if(wd + tw + 1 > SCREEN_WIDTH) {
            tmp[tw++] = /* [ */ ']';
            tmp[tw]   = '\0';
            fprintf(fp, "%s\n      ", tmp);
            wd   = 6;
            *tmp = '\0';
        }
        ++travp;
    }
    if(*tmp != '\0') {
        tmp[tw++] = ']';
        tmp[tw]   = '\0';
        wd += fprintf(fp, "%s", tmp);
    }

    // Any other args
    travp = cbi->table;
    while(travp->ln != NULL || travp->sh != 0) {
        if(travp->ln == NULL && travp->sh != 0 && takes_void(travp->type)) {
            ++travp;
            continue;
        }

        opt_to_text(travp, tmp, sizeof(tmp), W_SPACE | W_BRACKET | W_ALT);
        if(wd + strlen(tmp) > SCREEN_WIDTH) {
            fprintf(fp, "\n      ");
            wd = 6;
        }
        wd += fprintf(fp, "%s", tmp);
        ++travp;
    }

    fprintf(fp, "\n");
    return;
}

EXPORT_SYMBOL void HX_getopt_usage_cb(const struct HXoptcb *cbi)
{
    HX_getopt_usage(cbi, stdout);
    exit(EXIT_SUCCESS);
    return;
}

//-----------------------------------------------------------------------------
/*
Multiple possibilities for shconfig_pv()'s flags:

SHCONF_ONE
  Parsing will stop after one file has been successfully parsed. This allows
  for a "hardcoded+personal" configuration style. If you had

    const char *pv[] = {".", "/usr/local/etc", "/etc", NULL};
    HX_shconfig_pv(pv, "app.conf", table, flags | SHCONF_ONE);

  only the first (from left to right) successfully parsed will be read.

  To build the current options from default files "hardcoded+default+personal",
  this would be the answer:

    const char *pv[] = {"/etc", "/usr/local/etc", ".", NULL};
    HX_shconfig_pv(pv, "app.conf", table, flags);

  which reads all files, from left to right.
*/

EXPORT_SYMBOL int HX_shconfig(const char *file, const struct HXoption *table)
{
    struct HXoptcb cbi = {.table = table};
    char *reparse = NULL;
    hmc_t *ln = NULL;
    FILE *fp;

    if((fp = fopen(file, "r")) == NULL)
        return -errno;

    while(HX_getl(&ln, fp) != NULL) {
        char *lp = ln, *key, *val, *w;
        HX_chomp(ln);

 reparse_point:
        key = lp;
        // Next entry if comment, empty line or no value
        if(*lp == '#' || *lp == '\0' || (val = strchr(lp, '=')) == NULL)
            continue;

        while(isspace(*key))
            ++key;

        // Skip any whitespace directly before and after '='
        if((w = strchr(key, ' ')) != NULL && w < val)
            *w = '\0';

        *val++ = '\0';
        while(isspace(*val))
            ++val;

        // Handle escape codes and quotes, and assign to TAB entry
        reparse = shell_unescape(val);
        if((cbi.current = lookup_long(table, key)) == NULL)
            continue;

        cbi.tln = key;
        cbi.s   = val;
        do_assign(&cbi);

        if(reparse != NULL) {
            lp = reparse;
            goto reparse_point;
        }
    }

    hmc_free(ln);
    fclose(fp);
    return 1;
}

EXPORT_SYMBOL int HX_shconfig_pv(const char **path, const char *file,
  const struct HXoption *table, unsigned long flags)
{
    char buf[MAXFNLEN];
    int rv = 0;

    while(*path != NULL) {
        int v;
        snprintf(buf, sizeof(buf), "%s/%s", *path, file);
        v = HX_shconfig(buf, table);
        if(v > 0) {
            ++rv;
            if(flags & SHCONF_ONE)
                break;
        }
        ++path;
    }

    return rv;
}

EXPORT_SYMBOL void HX_shconfig_free(const struct HXoption *table)
{
    while(table->ln != NULL) {
        char **ptr = table->ptr;
        if(table->type == HXTYPE_STRING && ptr != NULL && *ptr != NULL)
            free(*ptr);
        ++table;
    }
    return;
}

//-----------------------------------------------------------------------------
static void do_assign(struct HXoptcb *cbi)
{
    const struct HXoption *opt = cbi->current;

    switch(opt->type & HXOPT_TYPEMASK) {
        case HXTYPE_NONE: {
            int *p;
            if((p = opt->ptr) != NULL) {
                p = opt->ptr;
                if(opt->type & HXOPT_INC)      ++*p;
                else if(opt->type & HXOPT_DEC) --*p;
                else                           *p = 1;
            }
            cbi->l = 1;
            CALL_CB;
            break;
        }
        case HXTYPE_IVAL:
            *static_cast(int *, opt->ptr) = cbi->l = opt->val;
            CALL_CB;
            break;
        case HXTYPE_SVAL:
            *reinterpret_cast(const char **, opt->ptr) = cbi->s = opt->sval;
            CALL_CB;
            break;
        case HXTYPE_BOOL: {
            int *p = opt->ptr;
            if((p = opt->ptr) != NULL)
                *p = strcasecmp(cbi->s, "yes") == 0 ||
                     strcasecmp(cbi->s, "on") == 0 ||
                     strcasecmp(cbi->s, "true") == 0 ||
                     strcmp(cbi->s, "1");
            break;
        }
        case HXTYPE_BYTE:
            *static_cast(unsigned char *, opt->ptr) = *cbi->s;
            CALL_CB;
            break;

        NTYPE_U(HXTYPE_UCHAR,  unsigned char)
        NTYPE_S(HXTYPE_CHAR,   char)
        NTYPE_U(HXTYPE_USHORT, unsigned short);
        NTYPE_S(HXTYPE_SHORT,  short);
        NTYPE_U(HXTYPE_UINT,   unsigned int);
        NTYPE_S(HXTYPE_INT,    int);
        NTYPE_U(HXTYPE_ULONG,  unsigned long);
        NTYPE_S(HXTYPE_LONG,   long);
#ifndef _MSC_VER
        NTYPE(HXTYPE_ULLONG,   unsigned long long, strtoull);
        NTYPE(HXTYPE_LLONG,    long long, strtoll);
#endif
        case HXTYPE_FLOAT:
            cbi->d = strtod(cbi->s, NULL);
            if(opt->ptr != NULL)
                *static_cast(float *, opt->ptr) = cbi->d;
            CALL_CB;
            break;
        case HXTYPE_DOUBLE:
            cbi->d = strtod(cbi->s, NULL);
            if(opt->ptr != NULL)
                *static_cast(double *, opt->ptr) = cbi->d;
            CALL_CB;
            break;
        case HXTYPE_STRING:
            if(opt->ptr != NULL)
                *static_cast(char **, opt->ptr) = HX_strdup(cbi->s);
            CALL_CB;
            break;
        case HXTYPE_STRDQ:
            HXdeque_push(opt->ptr, HX_strdup(cbi->s));
            CALL_CB;
            break;
        default:
            fprintf(stderr, "libHX-opt: illegal type %d\n",
             opt->type & HXOPT_TYPEMASK);
            break;
    }
    return;
}

static inline const struct HXoption *lookup_short(const struct HXoption *table,
  char opt)
{
    while(table->ln != NULL || table->sh != '\0') {
        if(table->sh == opt)
            return table;
        ++table;
    }
    return NULL;
}

static inline const struct HXoption *lookup_long(const struct HXoption *table,
  const char *key)
{
    while(table->ln != NULL || table->sh != '\0') {
        if(table->ln != NULL && strcmp(table->ln, key) == 0)
            return table;
        ++table;
    }
    return NULL;
}

static void opt_to_text(const struct HXoption *opt, char *buf, size_t len,
  unsigned int flags)
{
    const char *alt, *htyp = (opt->htyp != NULL) ? opt->htyp : "ARG";
    size_t i = 0;
    char equ;

    if(flags & W_SPACE)   buf[i++] = ' ';
    if(flags & W_BRACKET) buf[i++] = '['; //]
    if(flags & W_ALT) {
        alt = "|";
        equ = (flags & W_EQUAL) ? '=' : ' ';
    } else {
        alt = ", ";
        equ = '=';
    }

    if(opt->ln == NULL) {
        buf[i++] = '-';
        buf[i++] = opt->sh;
        if(!takes_void(opt->type))
            i += snprintf(buf + i, len - i, " %s", htyp);
    } else {
        if(opt->sh == 0) {
            if(takes_void(opt->type))
                i += snprintf(buf + i, len - i, "--%s", opt->ln);
            else
                i += snprintf(buf + i, len - i, "--%s=%s", opt->ln, htyp);
        } else {
            if(takes_void(opt->type))
                i += snprintf(buf + i, len - i, "-%c%s--%s",
                 opt->sh, alt, opt->ln);
            else
                i += snprintf(buf + i, len - i, "-%c%s--%s%c%s",
                 opt->sh, alt, opt->ln, equ, htyp);
        }
    }

    if(flags & W_BRACKET) buf[i++] = ']';
    buf[i] = '\0';
    return;
}

static void print_indent(const char *msg, size_t ind, size_t pos, FILE *fp)
{
    size_t rest = SCREEN_WIDTH - pos;
    char *p;
    while(1) {
        if(strlen(msg) < rest) {
            fprintf(fp, "%s", msg);
            break;
        }
        if((p = HX_strbchr(msg, msg + rest, ' ')) == NULL) {
            fprintf(fp, "%s", msg);
            break;
        }
        fprintf(fp, "%.*s", static_cast(int, p - msg), p);
        msg  = p + 1;
        rest = ind;
    }
    fprintf(fp, "\n");
    return;
}

static inline char *shell_unescape(char *o)
{
    char *i = o, quot = '\0';
    while(*i != '\0') {
        if(quot == '\0') {
            switch(*i) {
                case '"':
                case '\'':
                    quot = *i++;
                    continue;
                case '\\':
                    if(*++i == '\\')
                        *o++ = *i;
                    continue;
                case ';':
                    *o = '\0';
                    return i + 1;
                default:
                    *o++ = *i++;
                    continue;
            }
        }
        if(*i == quot) {
            quot = 0;
            ++i;
            continue;
        } else if(*i == '\\') {
            *o++ = *++i;
            ++i;
            continue;
        }
        *o++ = *i++;
    }
    *o = '\0';
    return NULL;
}

static inline unsigned int takes_void(unsigned int t)
{
    t &= HXOPT_TYPEMASK;
    return t == HXTYPE_NONE || t == HXTYPE_IVAL || t == HXTYPE_SVAL;
}

//=============================================================================
