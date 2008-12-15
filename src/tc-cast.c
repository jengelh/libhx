/*
 *	Testing for compile error in the cast helpers
 *	written by Jan Engelhardt
 *	this program is released in the Public Domain
 */
#include <math.h>
#include <stdio.h>
#include <libHX/defs.h>
#define UNUSED __attribute__((unused))

int main(void)
{
	const char *si_00 UNUSED = "foo";
	char *si_01 UNUSED = const_cast1(char *, si_00);
	signed char *si_02 UNUSED = signed_cast(signed char *, si_01);
	unsigned char *si_03 UNUSED = signed_cast(unsigned char *, si_01);
	const signed char *si_04 UNUSED = signed_cast(const signed char *, si_00);
	const unsigned char *si_05 UNUSED = signed_cast(const unsigned char *, si_00);
	si_00 = signed_cast(const char *, si_05);

	void *sr_00 = reinterpret_cast(void *, static_cast(long, 8));
	int sr_01 = reinterpret_cast(long, sr_00);
	void *sr_02 = reinterpret_cast(void *, reinterpret_cast(int, &si_00));
	printf("sr: %p %u; %p[%p]\n", sr_00, sr_01, sr_02, &si_00);

	double st_00 = sqrt(static_cast(int,
		10 * sqrt(static_cast(double, 3) / 4)));
	printf("st: %f\n", st_00);

	const int *co_00 UNUSED = NULL;
	int *co_01 UNUSED = const_cast1(int *, co_00);
	co_00 = co_01;

	const int **co_02 UNUSED = NULL;
	int **co_03 UNUSED = const_cast2(int **, co_02);
	int *const *co_04 UNUSED = const_cast2(int *const *, co_02);
	const int *const *co_05 UNUSED = const_cast2(const int *const *, co_02);
	co_02 = const_cast2(const int **, co_05);
	co_04 = const_cast2(int *const *, co_05);

	const int *const *const *co_06 = NULL;
	int ***co_07 UNUSED = const_cast3(int ***,
		(printf("If this line is only printed once when the program "
		"is run, __builtin_choose_expr works as desired.\n"), co_06));

	return 0;
}
