#include <cstdio>
#include <iterator>
#include <vector>
#include <libHX/intdiff.hpp>
int main()
{
	std::vector<int> a{1, 2, 5};
	std::vector<long> b{2, 3, 4, 5};
	std::vector<float> comm, left, right;

	HX::set_intersect_diff(a.cbegin(), a.cend(), b.cbegin(), b.cend(),
		std::back_inserter(comm), std::back_inserter(left),
		std::back_inserter(right));
	for (auto e : comm)
		printf("%f,", e);
	printf("\n");
	for (auto e : left)
		printf("%f,", e);
	printf("\n");
	for (auto e : right)
		printf("%f,", e);
	printf("\n");
	return 0;
}
