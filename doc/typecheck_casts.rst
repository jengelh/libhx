===================
Type-checking casts
===================

The C++ language provides “new-style casts”, referring to the four
template-looking invocations ``static_cast<>``, ``const_cast<>``,
``reinterpret_cast<>`` and ``dynamic_cast<>``. No such blessing was given to
the C language, but still, even using macros that expand to the olde cast make
it much easier to find casts in source code and annotate why something was
casted, which is already an improvement. — Actually, it is possible to do a
some type checking, using some GCC extensions, which augments these macros from
their documentary nature to an actual safety measure.


``reinterpret_cast``
====================

``reinterpret_cast()`` maps directly to the old-style typecast,
``(type)(expr)``, and causes the bit pattern for the ``expr`` rvalue to be
“reinterpreted” as a new type. You will notice that “reinterpret” is the
longest of all the ``*_cast`` names, and can easily cause lines to grow beyond
80 columns (the good maximum in many style guides). As a side effect, it is a
good indicator that something potentially dangerous might be going on, for
example converting intergers from/to pointer.

.. code-block:: c

	#include <libHX/defs.h>

	int i;
	/* Tree with numeric keys */
	tree = HXhashmap_init(0);
	for (i = 0; i < 6; ++i)
		HXmap_add(tree, reinterpret_cast(void *,
			  static_cast(long, i)), my_data);


``signed_cast``
===============

This tag is for annotating that the cast was solely done to change the
signedness of pointers to char — and only those. No integers etc. The intention
is to facilitate working with libraries that use ``unsigned char *`` pointers,
such as libcrypto and libssl (from the OpenSSL project) or libxml2, for
example. See table [tab:defs-signed_cast] for the allowed conversions. C++ does
not actually have a ``signed_cast<>``, and one would have to use
``reinterpret_cast<>`` to do the conversion, because ``static_cast<>`` does not
allow conversion from ``const char *`` to ``const unsigned char *``, for
example. (libHX's ``static_cast()`` would also throw at least a compiler
warning about the different signedness.) This is where signed_cast comes in.
(libHX provides a ``signed_cast<>`` for C++ though.)

.. table :: Accepted conversions for ``signed_cast()``

+-----------------------+----+-----+-----+-----+------+------+
|       From \ To       | c* | sc* | uc* | Cc* | Csc* | Cuc* |
+=======================+====+=====+=====+=====+======+======+
|        char *         | ok | ok  | ok  | ok  | ok   | ok   |
+-----------------------+----+-----+-----+-----+------+------+
|     signed char *     | ok | ok  | ok  | ok  | ok   | ok   |
+-----------------------+----+-----+-----+-----+------+------+
|    unsigned char *    | ok | ok  | ok  | ok  | ok   | ok   |
+-----------------------+----+-----+-----+-----+------+------+
|     const char *      | –  | –   | -   | ok  | ok   | ok   |
+-----------------------+----+-----+-----+-----+------+------+
|  const signed char *  | –  | –   | –   | ok  | ok   | ok   |
+-----------------------+----+-----+-----+-----+------+------+
| const unsigned char * | –  | –   | –   | ok  | ok   | ok   |
+-----------------------+----+-----+-----+-----+------+------+


``static_cast``
===============

Just like C++'s ``static_cast<>``, libHX's ``static_cast()`` verifies that
``expr`` can be implicitly converted to the new type (by a simple ``b = a``).
Such is mainly useful for forcing a specific type, as is needed in varargs
functions such as ``printf``, and where the conversion actually incurs other
side effects, such as truncation or promotion:

.. code-block:: c

	/* Convert to a type printf knows about */
	uint64_t x = something;
	printf("%llu\n", static_cast(unsigned long long, x));

Because there is no format specifier for ``uint64_t`` for ``printf`` (well yes,
there is ``PRIu64``), a conversion to an accepted type is necessary to not
cause undefined behavior. Code that does, for example, ``printf("%u")`` on a
``long`` only happens to work on architectures where ``sizeof(unsigned int) ==
sizeof(unsigned long)``, such as i386. On x86_64, an ``unsigned long`` is
usually twice as big as an ``unsigned int``, so that 8 bytes are pushed onto
the stack, but printf only unshifts 4 bytes because the developer indicated
``%u``, leading to misreading the next variable on the stack.

.. code-block:: c

	/* Force promotion */
	double a_quarter = static_cast(double, 1) / 4;

Were ``1`` not promoted to double, the result in ``q`` would be zero because
``1/4`` is just an integer division, yielding zero. By making one of the
operands a floating-point quantity, the compiler will instruct the FPU to
compute the result. Of course, one could have also written ``1.0`` instead of
``static_cast(double, 1)``, but this is left for the programmer to decide which
style s/he prefers.

.. code-block:: c

	/* Force truncation before invoking second sqrt */
	double f = sqrt(static_cast(int, 10 * sqrt(3.0 / 4)));

And here, the conversion from ``double`` to ``int`` incurs a (wanted)
truncation of the decimal fraction, that is, rounding down for positive
numbers, and rounding up for negative numbers.

Allowed conversions
-------------------

* Numbers

  Conversion between numeric types, such as ``char``, ``short``, ``int``,
  ``long``, ``long long``, ``intN_t``, both their signed and unsigned variants,
  ``float`` and ``double``.

* Generic Pointer

  Conversion from ``type *`` to and from ``void *``. (Where type may very
  well be a type with further indirection.)

* Generic Pointer (const)

  Conversion from ``const type *`` to and from ``const void *``.

Limitations
-----------

Because the implementation of our ``static_cast`` involves a C99 compound
literals and those are not constant expressions, ``static_cast`` cannot be used
in such contexts. (Cf. `GCC issue 105510
<https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105510#c3>`_).

.. code-block:: c

	static const int a = static_cast(int, 1U);

Furthermore, because an implicit assignment is used in the implementation, it
can trigger `-Wsign-conversion` warnings.


``const_cast``
==============

const_cast allows to add or remove “const” qualifiers from the
type a pointer is pointing to. Due to technical limitations, it
could not be implemented to support arbitrary indirection.
Instead, const_cast comes in three variants, to be used for
indirection levels of 1 to 3:

* ``const_cast1(type *, expr)`` with ``typeof(expr) = type *``.
  (Similarly for any combinations of const.)

* ``const_cast2(type **, expr)`` with ``typeof(expr) = type **`` (and all
  combinations of const in all possible locations).
 
* ``const_cast3(type ***, expr)`` with ``typeof(expr) = type ***`` (and all
  combinations...).

As indirection levels above 3 are really unlikely[#f3], having only these three
type-checking cast macros was deemed sufficient. The only place where libHX
even uses a level‑3 indirection is in the option parser.

.. [#t3] See “Three Star Programmer”

Conversion is permitted when expression and target type are from the table.

It is currently not possible to use const_cast1/2/3 on pointers to structures
whose member structure is unknown.
