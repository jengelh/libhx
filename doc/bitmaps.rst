=======
Bitmaps
=======

.. code-block:: c

	#include <libHX/misc.h>

	size_t HXbitmap_size(type array, unsigned int bits);
	void HXbitmap_set(type *bmap, unsigned int bit);
	void HXbitmap_clear(type *bmap, unsigned int bit);
	bool HXbitmap_test(type *bmap, unsigned int bit);

All of these four are implemented as macros, so they can be used with any
integer type that is desired to be used.

``HXbitmap_size``
	Returns the amount of ``type``-based integers that would be needed to
	hold an array of the requested amount of bits.

``HXbitmap_set``
	Sets the specific bit in the bitmap.

``HXbitmap_clear``
	Clears the specific bit in this bitmap.

``HXbitmap_test``
	Tests for the specific bit and returns true if it is set, otherwise
	false.


Example
=======

.. code-block:: c

	#include <stdlib.h>
	#include <string.h>
	#include <libHX/misc.h>

	int main(void)
	{
		unsigned long bitmap[HXbitmap_size(unsigned long, 128)];

		memset(bitmap, 0, sizeof(bitmap));
		HXbitmap_set(bitmap, 49);
		return HXbitmap_get(bitmap, HX_irand(0, 128)) ?
		       EXIT_SUCCESS : EXIT_FAILURE;
	}
