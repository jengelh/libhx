=======================
Miscellaneous functions
=======================

.. code-block:: c

	#include <libHX/misc.h>

	int HX_ffs(unsigned long z);
	int HX_fls(unsigned long z);
	void HX_hexdump(FILE *fp, const void *ptr, unsigned int len);

	void HX_zvecfree(char **);
	unsigned int HX_zveclen(const char *const *);

``HX_ffs``
	Finds the first (lowest-significant) bit in a value and returns its
	position, or ``-1`` to indicate failure.

``HX_fls``
	Finds the last (most-significant) bit in a value and returns its
	position, or ``-1`` to indicate failure.

``HX_hexdump``
	Outputs a nice pretty-printed hex and ASCII dump to the filedescriptor
	``fp``. ``ptr`` is the memory area, of which ``len`` bytes will be
	dumped.

``HX_zvecfree``
	Frees the supplied Z-vector array. (Frees all array elements from the
	first element to (excluding) the first ``NULL`` element.)

``HX_zveclen``
	Counts the number of array elements until the first ``NULL`` array
	element is seen, and returns this number.
