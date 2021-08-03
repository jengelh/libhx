==============
Random numbers
==============

Function overview
=================

.. code-block:: c

	#include <libHX/misc.h>

	int HX_rand(void);
	unsigned int HX_irand(unsigned int min, unsigned int max);
	double HX_drand(double min, double max);

``HX_rand``
	Retrieve the next random number.

``HX_irand``
	Retrieve the next random number and fold it such that *min <= n < max*.

``HX_drand``
	Retrieve the next random number and fold it such that *min <= n < max*.

Implementation information
==========================

``/dev/urandom`` will be used to seed the libc-level number generator.

``/dev/random`` is not used on Linux because it may block during read, and
/dev/urandom is just as good when there is entropy available. If you need
definitive PRNG security, perhaps use one from a crypto suite such as OpenSSL.
