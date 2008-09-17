/*
 *	libHX/other.c
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 1999 - 2008
 *
 *	This file is part of libHX. libHX is free software; you can
 *	redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2 or 3 of the License.
 */
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#if defined(_WIN32)
#	include <io.h>
#else
#	include <sys/types.h>
#	include <sys/wait.h>
#	include <unistd.h>
#endif
#include <libHX/misc.h>
#include "internal.h"

static int run_program(const char *, const char **, unsigned int);
#ifdef _WIN32
static int win32_system(const char **);
#endif

//-----------------------------------------------------------------------------
EXPORT_SYMBOL int HX_ffs(unsigned long n)
{
	int s = 0;
	if (n == 0)
		return -1;
	while ((n >>= 1) >= 1)
		++s;
	return s;
}

EXPORT_SYMBOL void HX_hexdump(FILE *fp, const void *vptr, unsigned int len)
{
	const unsigned char *ptr = vptr;
	unsigned int i, j;
	bool tty = isatty(fileno(fp));

	fprintf(fp, "Dumping %u bytes\n", len);
	for (i = 0; i < len / 16; ++i) {
		fprintf(fp, "%04x | ", i * 16);
		for (j = 0; j < 16; ++j)
			fprintf(fp, "%02x%c", *ptr++, (j == 7) ? '-' : ' ');
		ptr -= 16;
		fprintf(fp, "| ");
		for (j = 0; j < 16; ++j, ++ptr)
			if (isprint(*ptr))
				fprintf(fp, "%c", *ptr);
			else if (tty)
				fprintf(fp, "\e[31m.\e[0m"); // ]]
			else
				fprintf(fp, ".");
		fprintf(fp, "\n");
	}
	fprintf(fp, "%04x | ", i * 16);
	len -= i * 16;
	for (i = 0; i < len; ++i)
		fprintf(fp, "%02x%c", ptr[i], (i == 7) ? '-' : ' ');
	for (; i < 16; ++i)
		fprintf(fp, "   ");
	fprintf(fp, "| ");
	for (i = 0; i < len; ++i)
		if (isprint(ptr[i]))
			fprintf(fp, "%c", ptr[i]);
		else if (tty)
			fprintf(fp, "\e[31m.\e[0m"); // ]]
		else
			fprintf(fp, ".");
	fprintf(fp, "\n");
}

EXPORT_SYMBOL void HX_zvecfree(char **args)
{
	char **travp = args;
	while (*travp != NULL) {
		free(*travp);
		++travp;
	}
	free(args);
}

EXPORT_SYMBOL int HX_fsystem(unsigned int opts, const char *prog,
    const char *arg0, ...)
{
	int r;
	va_list ap;
	va_start(ap, arg0);
	r = HX_vfsystem(opts, prog, arg0, ap);
	va_end(ap);
	return r;
}

EXPORT_SYMBOL int HX_vfsystem(unsigned int opts, const char *prog,
    const char *arg0, va_list ap)
{
#define MAX_ARGS 255
	const char *dst_argv[MAX_ARGS+1];
	const char **src_argv = NULL, *ptr;
	unsigned int count = 0;

	if ((opts & (HX_FSYSTEM_ARGV | HX_FSYSTEM_ARGV1)) ==
	    (HX_FSYSTEM_ARGV | HX_FSYSTEM_ARGV1)) {
		fprintf(stderr, "libHX-fsystem: Cannot combine both "
		        "FSYSTEM_ARGV and FSYSTEM_ARGV1\n");
		return -EINVAL;
	}

	dst_argv[count++] = arg0;

	if (opts & (HX_FSYSTEM_ARGV | HX_FSYSTEM_ARGV1))
		src_argv = va_arg(ap, const char **);

	if (opts & HX_FSYSTEM_ARGV1) {
		/* [prog] [argv] [ap] */
		while (*src_argv != NULL) {
			if (count >= MAX_ARGS)
				break;
			dst_argv[count++] = *src_argv++;
		}
	}

	while ((ptr = va_arg(ap, const char *)) != NULL) {
		if (count >= MAX_ARGS)
			break;
		dst_argv[count++] = ptr;
	}

	if (opts & HX_FSYSTEM_ARGV) {
		/* [prog] [ap] [argv] */
		while (*src_argv != NULL) {
			if (count >= MAX_ARGS)
				break;
			dst_argv[count++] = *src_argv++;
		}
	}

	dst_argv[count++] = NULL;
	return run_program(prog, dst_argv, opts);
#undef MAX_ARGS
}

//-----------------------------------------------------------------------------
static int run_program(const char *prog, const char **argv, unsigned int opts)
{
#ifdef _WIN32
	int ret = win32_system(argv);
	if (opts & HX_FSYSTEM_EXEC)
		exit(ret);
	return ret;
#else
	if (opts & HX_FSYSTEM_EXEC) {
		return execve(prog, (char * const *)argv, NULL);
	} else {
		int status;
		pid_t pid;

		if ((pid = fork()) < 0)
			return -errno;
		else if (pid == 0)
			exit(execve(prog, (char * const *)argv, NULL));
		else
			return waitpid(pid, &status, 0);
	}
#endif
	return 0;
}

#ifdef _WIN32
int win32_system(const char **argv)
{
	char buf[4096], *wp = buf;
	size_t buflen = sizeof(buf), rem = buflen - 1;

	while (*argv != NULL) {
		const char *seg = *argv;
		if (rem < 3)
			break;
		*wp++ = ' ';
		*wp++ = '"';
		rem -= 2;
		do {
			size_t next = strcspn(seg, "\"");
			size_t sm = (rem < next) ? rem : next;
			strncpy(wp, seg, sm);
			rem -= sm;
			wp  += sm;
			if (rem == 0)
				break;
			seg += next;
		} while (*seg == '"');
		*wp++ = '"';
		--rem;
		++argv;
	}
	*wp++ = '\0';
	return system(buf);
}
#endif
