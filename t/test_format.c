#include "libHX.h"

int main(int argc, const char **argv)
{
	struct HXbtree *fmt = HXformat_init();
	HXformat_add(fmt, "USER", "jengelh", HXTYPE_STRING | HXFORMAT_IMMED);
	HXformat_add(fmt, "ARGC", &argc, HXTYPE_INT);
	HXformat_add(fmt, "ARGK", (const void *)argc, HXTYPE_INT | HXFORMAT_IMMED);
	HXformat_add(fmt, "ZERO", "", HXTYPE_STRING | HXFORMAT_IMMED);
	HXformat_add(fmt, "ONE", "1", HXTYPE_STRING | HXFORMAT_IMMED);
	++argc;
	HXformat_fprintf(fmt, stdout, "/%(HOME)/%(lower USER)/%(ARGC).%(ARGK)\n");
	HXformat_fprintf(fmt, stdout, ">%(ZERO)<\n");
	HXformat_fprintf(fmt, stdout, ">%(before=\"-o\" ZERO)<\n\n");
	HXformat_fprintf(fmt, stdout, ">%(ifempty=\"zero is empty\" ZERO)<\n");
	HXformat_fprintf(fmt, stdout, ">%(ifnempty=\"zero is not empty\" ZERO)<\n");
	HXformat_fprintf(fmt, stdout, ">%(ifempty=\"one is empty\" ONE)<\n");
	HXformat_fprintf(fmt, stdout, ">%(ifnempty=\"one is not empty\" ONE)<\n");
	return 0;
}
