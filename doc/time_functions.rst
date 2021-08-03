==============
Time functions
==============

Time in POSIX systems is represented in ``struct timespec``. This structure is
composed of two members: one integer for the number of full seconds in the time
value, and one integer for the number of nanoseconds that remain when
subtracting the full seconds from the time value. POSIX leaves it unspecified
how negative time is to be represented with this structure, so I have devised
an algebra for use with the same struct that gives negative time support.

Since integers often cannot store negative zero (due to e.g. use of 2s
complements in the language implementation), we will store the minus sign in
the nanosecond member if the integral second part is zero. This gives us the
property that we can test for negative time by looking for whether at least one
member of the structure is negative. Also, we want to avoid storing the minus
in both members to somewhat aid the pretty-printing construct often seen,

.. code-block:: c

	printf("%ld.%09ld\n", (long)ts.tv_sec, ts.tv_nsec);

The number of combinations of a (non-zero) negative number, zero and a
(non-zero) positive number is small, so we can actually just exhaustively list
them all.

+----------------+------------+
| Representation | Time value |
+================+============+
| {-1, -1}       |  illegal   |
+----------------+------------+
| {-1,  0}       |   -1.0 s   |
+----------------+------------+
| {-1,  1}       |   -1.1 s   |
+----------------+------------+
| { 0, -1}       |   -0.1 s   |
+----------------+------------+
| { 0,  0}       |    0.0 s   |
+----------------+------------+
| { 0,  1}       |    0.1 s   |
+----------------+------------+
| { 1, -1}       |  illegal   |
+----------------+------------+
| { 1,  0}       |    1.0 s   |
+----------------+------------+
| { 1,  1}       |    1.1 s   |
+----------------+------------+

Function list
=============

.. code-block:: c

	#include <libHX/misc.h>

	bool HX_timespec_isneg(const struct timespec *p);

	struct timespec *HX_timespec_neg(struct timespec *result,
		const struct timespec *p);

	struct timespec *HX_timespec_add(struct timespec *result,
		const struct timespec *p, const struct timespec *q);

	struct timespec *HX_timespec_sub(struct timespec *delta,
		const struct timespec *p, const struct timespec *q);

	struct timespec *HX_timespec_mul(struct timespec *delta,
		const struct timespec *p, int f);

	struct timespec *HX_timespec_mulf(struct timespec *delta,
		const struct timespec *p, double f);

	struct timeval *HX_timeval_sub(struct timeval *delta,
		const struct timeval *p, const struct timeval *q);

	int HX_time_compare(const struct stat *a, const struct stat *b,
		int mode);

``HX_timespec_isneg``
	Determines whether a timespec structure represents (non-zero) negative
	time.

``HX_timespec_neg``
	Computes the negation of the time specified by ``p``. ``result`` and
	``p`` may point to the same structure.

``HX_timespec_add``
	Calculates the sum of the two times specified by ``p`` and ``q``, which
	are of type ``struct timespec``. Any and all of ``result``, ``p`` and
	``q`` may point to the same structure.

``HX_timespec_sub``
	Calculates the difference between the two timepoints ``p`` and ``q``,
	which are of type ``struct timespec`` (nanosecond granularity).

``HX_timespec_mul``
	Multiplies the time quantum in ``p`` by ``f``.

``HX_timespec_mulf``
	Multiplies the time quantum in ``p`` by ``f``.

``HX_timeval_sub``
	Calculates the difference between the two timepoints ``p`` and ``q``,
	which are of type ``struct timeval`` (microsecnod granularity).

``HX_time_compare``
	Compares the timestamps from two struct stats. ``mode`` indicates which
	field is compared, which can either be ``'a'`` for the access time,
	``'c'`` for the inode change time, ``'m'`` for the modification time,
	or ``'o'`` for the creation time (where available). Returns a negative
	number if the time in ``a`` is less than ``b``, zero when they are
	equal, or a positive number greater than zero if ``a`` is greater than
	``b``.

The macros ``HX_TIMESPEC_FMT`` and ``HX_TIMESPEC_EXP`` can be used for passing
and printing a ``struct timespec`` using the ``*printf`` function family:

.. code-block:: c

	struct timespec p;
	clock_gettime(CLOCK_MONOTONIC, &p);
	printf("Now: " HX_TIMESPEC_FMT, HX_TIMESPEC_EXP(&p));

Similarly, ``HX_TIMEVAL_FMT`` and ``HX_TIMEVAL_EXP`` exist for the older
``struct timeval``.
