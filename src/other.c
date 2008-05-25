/*
 *	libHX/other.c
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 1999 - 2008
 *
 *	This file is part of libHX. libHX is free software; you can
 *	redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2 or 3 of the License.
 */
#include <errno.h>
#include <stdarg.h>
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
#include "libHX.h"

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
