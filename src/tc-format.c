/*
 *	formatter test program
 *	Copyright by Jan Engelhardt
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the WTF Public License version 2 or
 *	(at your option) any later version.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX/defs.h>
#include <libHX/init.h>
#include <libHX/option.h>

static const char *const fmt2_strings[] = {
	"HOME=%(env HOME)\n",
	"USER=%(upper %(lower %(env USER)))\n",
	"lower: %(lower foo, bar) %(lower %(TWOARG))\n",
	"substr-1: %(substr Hello World,0,5) %(substr Hello World,-5,5)\n",
	"substr-2: %(substr Hello World,-15,-6) %(substr Hello World,6)\n",
	"substr-3: %(substr Hello World,-99) <%(substr Hello World,12,2)>\n",
	"substr-4: <%(substr Hello World,-22,-11)> %(substr Hello World,-12,5)\n",
	"no-exp: %%(NOEXPANSION) %NOEXPANSION\n",
	"empty-1: <%()>\n",
	"empty-2: <%( )>\n",
	"empty-3: <%(dunno )>\n",
	"empty-4: <%(echo )>\n",
	"empty-5: <%(echo %())>\n",
	"empty-6: <%(echo %( ))>\n",
	"empty-7: <%(env )> <%(exec )> <%(if )> <%(if cond)> <%(lower )>\n",
	"empty-8: <%(shell )> <%(snl )> <%(upper )>\n",
	"basic: <%(ZERO)> <%(ONE)>\n",
	"recursive-var: <%(%(USER))>\n",
	"recursive-func: <%(%(env USER))>\n",
	"ignore-escape: %(echo A\\,B) %(echo A\\)B)\n",
	"quote-1: %(echo \"A,B\")\n",
	"quote-2: %(echo %(echo A,B),%(echo C,D),%(echo E F))\n",
	"quote-3: %(echo \"A)B\")\n",
	"quote-4: %(echo foo bar) %(echo foo\\ bar)\n",
	"unclosed-1: %(echo \"%(echo A\",B)\n",
	"unclosed-2: %(if X,Y,Z", /* ) */
	"nest-1: %(echo ()) %(echo %())\n",
	"nest-2: %(echo \\(A) %(echo \\)B)\n",
	"nest-3: %(echo \\)B\\() %(echo )B()\n",
	"if-1: %(if %(ZERO),,\"zero is empty\")\n",
	"if-2: %(if %(ZERO),\"zero is not empty\")\n",
	"if-3: %(if %(ONE),,\"one is empty\")\n",
	"if-4: %(if %(ONE),\"one is not empty\")\n",
	"if-5: %(if %(ONE),-o%(ONE))\n",
	"exec-1: %(exec uname -s)\n",
	"exec-2: %(shell uname -s)\n",
	"exec-3: %(snl %(shell uname -s))\n",
	NULL,
};

static void t_format(int argc)
{
	struct HXformat_map *fmt = HXformat_init();
	const char *const *s;

	HXformat_add(fmt, "/libhx/exec", NULL, HXFORMAT_IMMED);
	HXformat_add(fmt, "jengelh", "1337", HXTYPE_STRING | HXFORMAT_IMMED);
	HXformat_add(fmt, "USER", "jengelh", HXTYPE_STRING | HXFORMAT_IMMED);
	HXformat_add(fmt, "ARGC", &argc, HXTYPE_INT);
	HXformat_add(fmt, "ARGK", reinterpret_cast(const void *, static_cast(intptr_t, argc)), HXTYPE_INT | HXFORMAT_IMMED);
	HXformat_add(fmt, "ZERO", "", HXTYPE_STRING | HXFORMAT_IMMED);
	HXformat_add(fmt, "ONE", "1", HXTYPE_STRING | HXFORMAT_IMMED);
	HXformat_add(fmt, "TWOARG", "a, b", HXTYPE_STRING | HXFORMAT_IMMED);
	++argc;
	printf("# HXformat2\n");
	for (s = fmt2_strings; *s != NULL; ++s)
		HXformat_fprintf(fmt, stdout, *s);
	HXformat_free(fmt);
}

int main(int argc, const char **argv)
{
	int ret;

	ret = HX_init();
	if (ret <= 0) {
		fprintf(stderr, "HX_init: %s\n", strerror(-ret));
		return EXIT_FAILURE;
	}
	t_format(argc);
	HX_exit();
	return EXIT_SUCCESS;
}
