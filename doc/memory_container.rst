=================
Memory containers
=================

The HXmc series of functions provide scripting-like semantics for strings,
especially automatically resizing the buffer on demand. They can also be used
to store a binary block of data together with its length. (Hence the name: mc =
memory container.)

The benefit of using the HXmc functions is that one does not have to
meticulously watch buffer and string sizes anymore.

Improvement of string safety over time:

.. code-block:: c

	/* Step 1 */

	char buf[long_enough] = "helloworld";
	if (strlen(buf) + strlen(".txt") < sizeof(buf))
		strcat(s, ".txt"); /* may go over end of buffer */

	/* Step 2 */

	char buf[long_enough] = "helloworld";
	strlcat(s, ".txt", sizeof(buf)); /* may truncate */

	/* Step 3 */

	hxmc_t *buf = HXmc_strinit("helloworld");
	HXmc_strcat(&s, ".txt");

This makes it quite similar to the string operations (and append seems to be
the most commonly used one to me) supported in scripting languages that also do
without a size argument. The essential part of such memory containers is that
their internal (hidden) metadata structure contains the length of the memory
block in the container. For binary data this may be the norm, but for C-style
strings, the stored and auto-updated length field serves as an accelerator
cache. For more details, see ``HXmc_length``.

Of course, the automatic management of memory comes with a bit of overhead as
the string expands beyond its preallocated region. Such may be mitigated by
doing explicit (re)sizing.


Structural overview
===================

HXmc functions do not actually return a pointer to the memory container (e.g.
struct) itself, but a pointer to the data block. Conversely, input parameters
to HXmc functions will be the data block pointer. It is of type ``hxmc_t *``,
which is typedef'ed to ``char *`` and therefore has properties of a char
pointer. Pointer arithmetic is thus supported. It also means you can just pass
it to functions that take a ``char *`` without having to do a member access
like ``s.c_str()`` in C++. The drawback is that many functions operating on the
memory container need a ``hxmc_t **`` (a level-two indirection), because not
only does the memory block move, but also the memory container itself. This is
due to the implementation of the container metadata which immediately and
always precedes the writable memory block.

HXmc ensures that the data block is terminated by the \0 byte (unless you trash
it), so you do not have to, and of course, to be on the safe side. But, the
automatic \0 byte is not part of the region allocated by the user. That is,
when one uses the classic approach with ``malloc(4096)``, the user will have
control of 4096 bytes and has to stuff the \0 byte in there somehow on his own;
for strings this means the maximum string length is 4095. Requesting space for
a 4096-byte sized HXmc container gives you the possibility to use all 4096
bytes for the string, because HXmc provides a \0 byte.

By the way, ``hxmc_t`` is the only typedef in this entire library, to
distinguish it from regular ``char *`` that does not have a backing memory
cointainer.

Constructors, destructors
=========================

.. code-block:: c

	#include <libHX/string.h>

	hxmc_t *HXmc_strinit(const char *s);
	hxmc_t *HXmc_meminit(const void *ptr, size_t size);
	void HXmc_free(hxmc_t *s);
	void HXmc_zvecfree(hxmc_t **s);

``HXmc_strinit``
	Creates a new ``hxmc_t`` object from the supplied string and returns
	it.

``HXmc_meminit``
	Creates a new ``hxmc_t`` object from the supplied memory buffer of the
	given size and returns it. ``HXmc_meminit(NULL, len)`` may be used to
	obtain an empty container with a preallocated region of len bytes (zero
	is accepted for ``len``).

``HXmc_free``
	Frees the hxmc object.

``HXmc_zvecfree``
	Frees all hxmc objects in the ``NULL``-terminated array, and finally
	frees the array itself, similar to ``HX_zvecfree``.

Data manipulation
=================

Binary-based
------------

.. code-block:: c

	hxmc_t *HXmc_trunc(hxmc_t **mc, size_t len);HXmc_trunc
	hxmc_t *HXmc_setlen(hxmc_t **mc, size_t len);HXmc_setlen
	hxmc_t *HXmc_memcpy(hxmc_t **mc, const void *ptr, size_t len);
	hxmc_t *HXmc_memcat(hxmc_t **mc, const void *ptr, size_t len);
	hxmc_t *HXmc_mempcat(hxmc_t **mc, const void *ptr, size_t len);
	hxmc_t *HXmc_memins(hxmc_t **mc, size_t pos, const void *ptr, size_t len);
	hxmc_t *HXmc_memdel(hxmc_t **mc, size_t pos, size_t len);

When ``ptr`` is ``NULL``, each call behaves as if ``len`` would be ``zero``.
Specifically, no undefined behavior will result of the use of ``NULL``.

``HXmc_trunc``
	Truncates the container's data to ``len`` size. If ``len`` is greater
	than the current data size of the container, the length is in fact not
	updated, but a reallocation may be triggered, which can be used to do
	explicit allocation.

``HXmc_setlen``
	Sets the data length, doing a reallocation of the memory container if
	needed. The newly available bytes are uninitialized. Make use of this
	function when letting 3rd party functions write to the buffer, but it
	should not be used with ``HXmc_str*``.

``HXmc_memcpy``
	Truncates the container's data and copies ``len`` bytes from the memory
	area pointed to by ``ptr`` to the container.

``HXmc_memcat``
	Concatenates (appends) ``len`` bytes from the memory area pointed to by
	``ptr`` to the container's data.

``HXmc_mempcat``
	Prepends ``len`` bytes from the memory area pointed to by ``ptr`` to
	the container's data.

``HXmc_memins``
	Prepends ``len`` bytes from the memory area pointed to by ``ptr`` to
	the ``pos``'th byte of the container's data.

``HXmc_memdel``
	Deletes ``len`` bytes from the container beginning at position ``pos``.

In case of a memory allocation failure, the ``HXmc_*`` functions will return
``NULL``.

String-based
------------

The string-based functions correspond to their binary-based equivalents with a
len argument of strlen(s).

.. code-block:: c

	hxmc_t *HXmc_strcpy(hxmc_t **mc, const char *s);
	hxmc_t *HXmc_strcat(hxmc_t **mc, const char *s);
	hxmc_t *HXmc_strpcat(hxmc_t **mc, const char *s);
	hxmc_t *HXmc_strins(hxmc_t **mc, size_t pos, const char *s);

``HXmc_strcpy``
	Copies the string pointed to by ``s`` into the memory container given
	by ``mc``. If ``mc`` is ``NULL``, the memory container will be
	deallocated, that is, ``*mc`` becomes ``NULL``.

From auxiliary sources
----------------------

.. code-block:: c

	hxmc_t *HX_getl(hxmc_t **mc, FILE *fp);HX_getl

``HX_getl``
	Reads the next line from ``fp`` and store the result in the container.
	Returns ``NULL`` on error, or when end of file occurs while no
	characters have been read.

Container properties
--------------------

.. code-block:: c

	size_t HXmc_length(const hxmc_t **mc);

``HXmc_length``
	Returns the length of the memory container. This is not always equal to
	the actual string length. For example, if ``HX_chomp`` was used on an
	MC-backed string, ``strlen`` will return less than ``HXmc_length`` if
	newline control characters (``\r`` and ``\n``) were removed.
