/*
 *	Copyright © Jan Engelhardt, 2012
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the WTF Public License version 2 or
 *	(at your option) any later version.
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <libHX/defs.h>
#include <libHX/init.h>
#include <libHX/misc.h>
#include "internal.h"

typedef struct timespec *(*add_func_t)(struct timespec *,
	const struct timespec *, const struct timespec *);

static const int NANOSECOND = 1000000000;
static const long long NANOSECOND_LL = 1000000000;
static const unsigned int clock_id = CLOCK_THREAD_CPUTIME_ID;
static const unsigned int step = 1000;

static const struct timespec pairs[] = {
	{-1, 700000000}, {-1, 400000000}, {-1, 0},
	{0, -700000000}, {0, -400000000}, {0, 0},
	{0, 400000000}, {0, 700000000},
	{1, 0}, {1, 400000000}, {1, 700000000},
};

/*
 * Variant that uses full 64 bit division and is thus slower on
 * a handful of hardware.
 */
static struct timespec *HX_timespec_add_FDIV(struct timespec *r,
    const struct timespec *a, const struct timespec *b)
{
	long long p, q;

	p  = a->tv_sec * NANOSECOND_LL +
	     ((a->tv_sec >= 0) ? a->tv_nsec : -a->tv_nsec);
	q  = b->tv_sec * NANOSECOND_LL +
	     ((b->tv_sec >= 0) ? b->tv_nsec : -b->tv_nsec);

	p += q;
	r->tv_sec  = p / NANOSECOND;
	r->tv_nsec = p % NANOSECOND;
	if (r->tv_sec < 0 && r->tv_nsec < 0)
		r->tv_nsec = -r->tv_nsec;
	return r;
}

static void test_same(void)
{
	static const struct timespec zero = {0, 0};
	struct timespec r;
	unsigned int i;

	printf("# Test src==dst operand behavior\n");

	/* 1s */
	for (i = 0; i < ARRAY_SIZE(pairs); ++i) {
		r = pairs[i];
		printf("-(" HX_TIMESPEC_FMT ") = ", HX_TIMESPEC_EXP(&r));
		HX_timespec_neg(&r, &r);
		printf(HX_TIMESPEC_FMT "\n", HX_TIMESPEC_EXP(&r));
	}
	printf("\n");

	for (i = 0; i < ARRAY_SIZE(pairs); ++i) {
		r = pairs[i];
		printf(HX_TIMESPEC_FMT " + 0 = ", HX_TIMESPEC_EXP(&r));
		HX_timespec_add(&r, &r, &zero);
		printf(HX_TIMESPEC_FMT "\n", HX_TIMESPEC_EXP(&r));
	}
	printf("\n");

	for (i = 0; i < ARRAY_SIZE(pairs); ++i) {
		r = pairs[i];
		printf(HX_TIMESPEC_FMT " - 0 = ", HX_TIMESPEC_EXP(&r));
		HX_timespec_sub(&r, &r, &zero);
		printf(HX_TIMESPEC_FMT "\n", HX_TIMESPEC_EXP(&r));
	}
	printf("\n");

	/* 2s */
	for (i = 0; i < ARRAY_SIZE(pairs); ++i) {
		r = pairs[i];
		printf(HX_TIMESPEC_FMT " + " HX_TIMESPEC_FMT " = ",
		       HX_TIMESPEC_EXP(&r), HX_TIMESPEC_EXP(&r));
		HX_timespec_add(&r, &r, &r);
		printf(HX_TIMESPEC_FMT "\n", HX_TIMESPEC_EXP(&r));
	}
	printf("\n");

	for (i = 0; i < ARRAY_SIZE(pairs); ++i) {
		r = pairs[i];
		printf(HX_TIMESPEC_FMT " - " HX_TIMESPEC_FMT " = ",
		       HX_TIMESPEC_EXP(&r), HX_TIMESPEC_EXP(&r));
		HX_timespec_sub(&r, &r, &r);
		printf(HX_TIMESPEC_FMT "\n", HX_TIMESPEC_EXP(&r));
	}
	printf("\n");
}

static void print_sgn(const struct timespec *a)
{
	printf(HX_timespec_isneg(a) ? "[-]" : "[+]");
}

static void test_neg(void)
{
	const struct timespec *now;
	struct timespec then;

	printf("# Negation\n");
	for (now = pairs; now < pairs + ARRAY_SIZE(pairs); ++now) {
		HX_timespec_neg(&then, now);

		print_sgn(now);
		printf(HX_TIMESPEC_FMT " -> ", HX_TIMESPEC_EXP(now));
		print_sgn(&then);
		printf(HX_TIMESPEC_FMT "\n", HX_TIMESPEC_EXP(&then));
	}
	printf("\n");
}

static void print_op2(const struct timespec *r, const struct timespec *a,
    const char *op, const struct timespec *b)
{
	printf(HX_TIMESPEC_FMT " %s ", HX_TIMESPEC_EXP(a), op);
	printf(HX_TIMESPEC_FMT " = ", HX_TIMESPEC_EXP(b));
	printf(HX_TIMESPEC_FMT "\n", HX_TIMESPEC_EXP(r));
}

static void test_add(void)
{
	const struct timespec *a, *b;
	struct timespec r, s;

	printf("# Test addition behavior\n");
	for (a = pairs; a < pairs + ARRAY_SIZE(pairs); ++a) {
		for (b = pairs; b < pairs + ARRAY_SIZE(pairs); ++b) {
			HX_timespec_add(&r, a, b);
			print_op2(&r, a, "+N", b);
			HX_timespec_add_FDIV(&s, a, b);
			print_op2(&r, a, "+F", b);
			if (r.tv_sec != s.tv_sec || r.tv_nsec != s.tv_nsec)
				abort();
			HX_timespec_sub(&r, a, b);
			print_op2(&r, a, "- ", b);
			printf("----------\n");
		}
	}
	printf("\n");
}

static void test_adds_nz(time_t s, add_func_t fn)
{
	struct timespec a, b, r;

	a.tv_sec = s;
	for (a.tv_nsec = 0; a.tv_nsec < NANOSECOND;
	     a.tv_nsec += NANOSECOND / step)
	{
		b.tv_sec = -1;
		for (b.tv_nsec = 0; b.tv_nsec < NANOSECOND;
		     b.tv_nsec += NANOSECOND / step)
			(*fn)(&r, &a, &b);

		b.tv_sec = 0;
		for (b.tv_nsec = -NANOSECOND + NANOSECOND / step;
		     b.tv_nsec < NANOSECOND; b.tv_nsec += NANOSECOND / step)
			(*fn)(&r, &a, &b);

		b.tv_sec = 1;
		for (b.tv_nsec = 0; b.tv_nsec < NANOSECOND;
		     b.tv_nsec += NANOSECOND / step)
			(*fn)(&r, &a, &b);
	}
}

static void test_adds_z(add_func_t fn)
{
	struct timespec a, b, r;

	a.tv_sec = 0;
	for (a.tv_nsec = -NANOSECOND + NANOSECOND / step;
	     a.tv_nsec < NANOSECOND; a.tv_nsec += NANOSECOND / step)
	{
		b.tv_sec = -1;
		for (b.tv_nsec = 0; b.tv_nsec < NANOSECOND;
		     b.tv_nsec += NANOSECOND / step)
			(*fn)(&r, &a, &b);

		b.tv_sec = 0;
		for (b.tv_nsec = -NANOSECOND + NANOSECOND / step;
		     b.tv_nsec < NANOSECOND; b.tv_nsec += NANOSECOND / step)
			(*fn)(&r, &a, &b);

		b.tv_sec = 1;
		for (b.tv_nsec = 0; b.tv_nsec < NANOSECOND;
		     b.tv_nsec += NANOSECOND / step)
			(*fn)(&r, &a, &b);
	}
}

static void test_adds_1(const char *text, add_func_t fn)
{
	struct timespec start, delta;

	printf("%s", text);
	clock_gettime(clock_id, &start);
	test_adds_nz(-2, fn);
	test_adds_nz(-1, fn);
	test_adds_z(fn);
	test_adds_nz(1, fn);
	test_adds_nz(2, fn);
	clock_gettime(clock_id, &delta);
	HX_timespec_sub(&delta, &delta, &start);
	printf(HX_TIMESPEC_FMT "\n", HX_TIMESPEC_EXP(&delta));
}

static void test_adds(void)
{
	printf("# Test addition speed\n");
	test_adds_1("normal:  ", HX_timespec_add);
	test_adds_1("fulldiv: ", HX_timespec_add_FDIV);
	printf("\n");
}

int main(void)
{
	if (HX_init() <= 0)
		abort();

	test_same();
	test_neg();
	test_add();
	test_adds();
	HX_exit();
	return EXIT_SUCCESS;
}
