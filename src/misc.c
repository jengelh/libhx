/*
 *	Miscellaneous functions
 *	Copyright Jan Engelhardt, 1999-2010
 *
 *	This file is part of libHX. libHX is free software; you can
 *	redistribute it and/or modify it under the terms of the GNU Lesser
 *	General Public License as published by the Free Software Foundation;
 *	either version 2.1 or (at your option) any later version.
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libHX/ctype_helper.h>
#include <libHX/misc.h>
#include "internal.h"

EXPORT_SYMBOL int HX_ffs(unsigned long n)
{
	int s = 0;
	if (n == 0)
		return -1;
	while ((n >>= 1) >= 1)
		++s;
	return s;
}

EXPORT_SYMBOL int HX_fls(unsigned long n)
{
	int i;
	for (i = 31; i >= 0; --i)
		if (n & (1 << i))
			return i;
	return -1;
}

static __inline__ void hexdump_ascii(FILE *fp, unsigned char c, bool tty)
{
	static const unsigned char ct_char[] = "31", up_char[] = "34";

	if (HX_isprint(c))
		fprintf(fp, "%c", c);
	else if (tty && c == 0)
		fprintf(fp, "\e[%sm@\e[0m", up_char); // ]]
	else if (tty && c < 32)
		fprintf(fp, "\e[%sm%c\e[0m", ct_char, '@' + c); // ]]
	else if (tty)
		fprintf(fp, "\e[%sm.\e[0m", up_char); // ]]
	else
		fprintf(fp, ".");
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
		for (j = 0; j < 16; ++j)
			hexdump_ascii(fp, *ptr++, tty);
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
		hexdump_ascii(fp, *ptr++, tty);
	fprintf(fp, "\n");
}

EXPORT_SYMBOL void HX_zvecfree(char **args)
{
	char **travp;
	for (travp = args; *travp != NULL; ++travp)
		free(*travp);
	free(args);
}
