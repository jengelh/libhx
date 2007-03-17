#include "libHX.h"

int main(int argc, const char **argv)
{
	struct HXbtree *fmt = HXformat_init();
	HXformat_add(fmt, "USER", "jengelh", HXTYPE_STRING | HXFORMAT_IMMED);
	HXformat_add(fmt, "ARGC", &argc, HXTYPE_INT);
	HXformat_add(fmt, "ARGK", (const void *)argc, HXTYPE_INT | HXFORMAT_IMMED);
	HXformat_add(fmt, "ZERO", "", HXTYPE_STRING | HXFORMAT_IMMED);
	++argc;
	HXformat_fprintf(fmt, stdout, "/%(HOME)/%(lower USER)/%(ARGC).%(ARGK)\n");
	HXformat_fprintf(fmt, stdout, "%(ZERO)");
	HXformat_fprintf(fmt, stdout, "%(before=\"-o\" ZERO)");
	return 0;
}
