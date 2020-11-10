#ifndef LIBHX_INTDIFF_HPP
#define LIBHX_INTDIFF_HPP 1
#include <algorithm>
namespace HX {
template<typename AIter, typename BIter, typename XIter,
         typename YIter, typename ZIter>
void set_intersect_diff(AIter a_first, AIter a_last, BIter b_first,
    BIter b_last, XIter xptr, YIter yptr, ZIter zptr)
{
	while ((a_first != a_last) && (b_first != b_last)) {
		if (*a_first < *b_first)
			*xptr++ = *a_first++;
		else if (*b_first < *a_first)
			*yptr++ = *b_first++;
		else {
			*zptr++ = *a_first++;
			++b_first;
		}
	}
	std::copy(a_first, a_last, xptr);
	std::copy(b_first, b_last, yptr);
}
}
#endif
