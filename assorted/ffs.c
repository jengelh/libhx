#include <libHX.h>

int HX_ffs(unsigned long n)
{
	register int s = 0;
	if(n == 0)
		return -1;
	while((n >>= 1) >= 1)
		++s;
	return s;
}
