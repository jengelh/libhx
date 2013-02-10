/*
 *	Behavior Correctness Test for HX_strquote
 *	Copyright © Jan Engelhardt, 2013
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the WTF Public License version 2 or
 *	(at your option) any later version.
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <libHX/string.h>

static const char input1[]   = "\"Good\" ol' \\'escaped\\' strings";
static const char output1a[] = "\"Good\" ol\\' \\\\\\'escaped\\\\\\' strings";
static const char output1b[] = "\\\"Good\\\" ol' \\\\'escaped\\\\' strings";
static const char input2[]   = "<p style=\"height: 1;\">Foo &amp; \"bar\"</p>";
static const char output2[]  =
	"&lt;p style=&quot;height: 1;&quot;&gt;Foo &amp;amp; &quot;bar&quot;&lt;/p&gt;";
static const char input3[]   = " #o=foo(*),ba\\r ";
static const char output3a[] = " #o=foo\\28\\2A\\29,ba\\5Cr ";
static const char output3b[] = "\\20\\23o\\3Dfoo(*)\\2Cba\\5Cr\\20";
static const char output3c[] = "ICNvPWZvbygqKSxiYVxyIA==";

static int test(const char *input, unsigned int mode, const char *expect)
{
	char *output = HX_strquote(input, mode, NULL);

	if (output == NULL) {
		fprintf(stderr, "HX_strquote returned NULL\n");
		return EXIT_FAILURE;
	}
	if (strcmp(output, expect) != 0) {
		fprintf(stderr, "Input: %s\nOutput: %s\nExpected: %s\n",
		        input, output, expect);
		free(output);
		return EXIT_FAILURE;
	}
	free(output);
	return EXIT_SUCCESS;
}

int main(void)
{
#define tst(a, b, c) \
	do { \
		int ret = test((a), (b), (c)); \
		if (ret != EXIT_SUCCESS) \
			return ret; \
	} while (false);

	if (HX_strquote(input1, ~0U, NULL) != NULL)
		return EXIT_FAILURE;
	tst(input1, HXQUOTE_SQUOTE, output1a);
	tst(input1, HXQUOTE_DQUOTE, output1b);
	tst(input2, HXQUOTE_HTML, output2);
	tst(input3, HXQUOTE_LDAPFLT, output3a);
	tst(input3, HXQUOTE_LDAPRDN, output3b);
	tst(input3, HXQUOTE_BASE64, output3c);
	return 0;
#undef tst
}
