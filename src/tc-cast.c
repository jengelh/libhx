/*
 *	Testing for compile error in the cast helpers
 *	written by Jan Engelhardt
 *	this program is released in the Public Domain
 */
#define HXDEV_NEW_CAST
#include <libHX/defs.h>

int main(void)
{
	const char *a = "foo";

	char *c __attribute__((unused)) =
		static_cast(char *, const_cast(void *, a));

	signed char *d __attribute__((unused)) =
		signed_cast(signed char *, c);

	unsigned char *e __attribute__((unused)) =
		signed_cast(unsigned char *, c);

	return 0;
}
