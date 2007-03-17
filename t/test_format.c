#include "libHX.h"

int main(int argc, const char **argv)
{
	struct HXbtree *fmt = HXformat_init();
	HXformat_add(fmt, "USER", "jengelh", HXTYPE_STRING);
	HXformat_add(fmt, "ARGC", &argc, HXTYPE_INT);
	HXformat_add(fmt, "ARGK", (const void *)argc, HXTYPE_INT | HXFORMAT_IMMED);
	++argc;
	HXformat_fprintf(fmt, stdout, "/%{HOME}/%{lower USER}/%{ARGC}.%{ARGK}\n");
	return 0;
}
