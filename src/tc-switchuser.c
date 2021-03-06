/*
 *	Copyright Jan Engelhardt
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the WTF Public License version 2 or
 *	(at your option) any later version.
 */
#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif
#if defined(HAVE_INITGROUPS)
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libHX/defs.h>
#include <libHX/option.h>
#include <libHX/proc.h>

static char *user_name, *group_name;
static const struct HXoption options_table[] = {
	{.sh = 'u', .type = HXTYPE_STRING, .ptr = &user_name},
	{.sh = 'g', .type = HXTYPE_STRING, .ptr=  &group_name},
	HXOPT_TABLEEND,
};

int main(int argc, const char **argv)
{
	HX_getopt(options_table, &argc, &argv, HXOPT_USAGEONERR);
	const char *user = user_name != NULL ? user_name : "-";
	const char *group = group_name != NULL ? group_name : "-";
	switch (HXproc_switch_user(user_name, group_name)) {
	case HXPROC_USER_NOT_FOUND:
		if (user_name == NULL)
			abort(); /* impossible outcomes */
		printf("No such user \"%s\": %s\n", user_name, strerror(errno));
		break;
	case HXPROC_GROUP_NOT_FOUND:
		if (group_name == NULL || *group_name == '\0')
			abort(); /* impossible outcome */
		printf("No such group \"%s\": %s\n", group_name, strerror(errno));
		break;
	case HXPROC_SETUID_FAILED:
		printf("setuid %s: %s\n", user, strerror(errno));
		break;
	case HXPROC_SETGID_FAILED:
		printf("setgid %s: %s\n", group, strerror(errno));
		break;
	case HXPROC_INITGROUPS_FAILED:
		printf("initgroups for %s: %s\n", user, strerror(errno));
		break;
	case HXPROC_SU_NOOP:
		printf("No action was performed./User identity already reached.\n");
		/* fallthrough */
	case HXPROC_SU_SUCCESS: {
		gid_t list[64] = {-1};
		int numgroups = getgroups(ARRAY_SIZE(list), list);
		printf("Identity now: uid %lu euid %lu gid %lu egid %lu\n",
		       static_cast(unsigned long, getuid()),
		       static_cast(unsigned long, geteuid()),
		       static_cast(unsigned long, getgid()),
		       static_cast(unsigned long, getegid()));
		printf("Secondary groups:");
		for (int i = 0; i < numgroups; ++i)
			printf(" %lu", static_cast(unsigned long, list[i]));
		printf("\n");
		break;
	}
	}
	return EXIT_SUCCESS;
}
#endif
