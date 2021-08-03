Initialization
==============

.. code-block:: c

	#include <libHX/init.h>
	
	int HX_init(void);
	void HX_exit(void);

Before using the library's functions, ``HX_init`` must be called. This function
will initialize any needed state libHX needs for itself, if any. It is designed
to be invoked multiple times, such as for example, from different libraries
linking to libHX itself, and will refcount. On success, >0 is returned. If
there was an error, it will return a negative error code or zero. ``HX_exit``
is the logical counterpart of notifying that the library is no longer used.

