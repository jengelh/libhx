/*
 *	Copyright © Jan Engelhardt, 2008-2009
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 3 or
 *	any later version as published by the Free Software Foundation.
 */
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX/misc.h>
#include <libHX/string.h>
#include "config.h"

static void ds_hexdump(const void *buf, unsigned int len)
{
	const unsigned char *bp = buf;
	while (len-- > 0)
		printf("%02x", *bp++);
}

static void ds_post_file(const char *file, const struct stat *sb)
{
#if defined(HAVE_STRUCT_STAT_ST_MTIMESPEC)
	ds_hexdump(&sb->st_mtimensec, sizeof(sb->st_mtimensec));
#elif defined(HAVE_STRUCT_STAT_ST_MTIM)
	ds_hexdump(&sb->st_mtim, sizeof(sb->st_mtim));
#elif defined(HAVE_STRUCT_STAT_ST_MTIMESPEC)
	ds_hexdump(&sb->st_mtimespec, sizeof(sb->st_mtimespec));
#elif defined(HAVE_STRUCT_STAT_ST_MTIME)
	ds_hexdump(&sb->st_mtime, sizeof(sb->st_mtime));
#endif
	printf(" %s\n", file);
}

static int ds_file(const char *file)
{
	struct stat sb;

	memset(&sb, 0, sizeof(sb));
	if (lstat(file, &sb) < 0) {
		fprintf(stderr, "stat: %s: %s\n", file, strerror(errno));
		return 2;
	}
	if (S_ISDIR(sb.st_mode))
		return 0;
	ds_post_file(file, &sb);
	return 0;
}

static int ds_dir(const char *dir)
{
	int ret, final_ret = 0;
	const char *dentry;
	hxmc_t *full;
	void *dh;

	if ((dh = HXdir_open(dir)) == NULL) {
		fprintf(stderr, "Cannot access %s: %s\n", dir, strerror(errno));
		return 2;
	}

	full = HXmc_strinit(dir);
	while ((dentry = HXdir_read(dh)) != NULL) {
		HXmc_trunc(&full, strlen(dir));
		HXmc_strcat(&full, "/");
		HXmc_strcat(&full, dentry);
		if ((ret = ds_file(full)) != 0 && final_ret == 0)
			final_ret = ret;
	}

	HXdir_close(dh);
	return final_ret;
}

static int ds_proc(const char *object)
{
	struct stat sb;

	if (lstat(object, &sb) < 0) {
		fprintf(stderr, "stat: %s: %s\n", object, strerror(errno));
		return 2;
	}
	if (S_ISDIR(sb.st_mode))
		return ds_dir(object);

	ds_post_file(object, &sb);
	return 0;
}

int main(int argc, const char *const *argv)
{
	int ret, final_ret = EXIT_SUCCESS;
	const char *const *dir_p;
	const char *dir;

	for (dir_p = ++argv, dir = *argv; *dir_p != NULL; dir = *++dir_p)
		if ((ret = ds_proc(dir)) != 0 && final_ret == 0)
			final_ret = ret;

	return final_ret;
}
