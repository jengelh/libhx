======
Macros
======

All macros in this section are available through ``#include <libHX/defs.h>``.

Preprocessor
============

.. code-block:: c

	#define HX_STRINGIFY(s)HX_STRINGIFY

Transforms the expansion of the argument ``s`` into a C string.


Sizes
=====

.. code-block:: c

	#define HXSIZEOF_Z16
	#define HXSIZEOF_Z32
	#define HXSIZEOF_Z64

Expands to the size needed for a buffer (including ``\0``) to hold the base-10
string representation of 16‑, 32‑ or 64‑bit integer (either signed or
unsigned), respectively.

Locators
========

.. code-block:: c

	output_type *containerof(input_type *ptr, output_type, member);

	size_t HXsizeof_member(struct_type, member);
	output_type HXtypeof_member(struct_type, member);

``containerof`` will return a pointer to the struct in which ``ptr`` is
contained as the given member. (In C++, it is required that the encompassing
``output_type`` has so-called "standard layout", but to date I have never found
an implementation where this matters.)

.. code-block:: c

	struct foo {
		int bar;
		int baz;
	};

	static void test(int *ptr)
	{
		struct foo *self = containerof(baz, struct foo, baz);
	}

``HXsizeof_member`` and ``HXtypeof_member`` are shortcuts (mainly for the C
language) to get the size or type of a named member in a given struct:

.. code-block:: c

	char padding[FIELD_SIZEOF(struct foo, baz)];

In C++, one can simply use ``sizeof(foo::baz)`` and ``decltype(foo::baz)``.


Array size
==========

.. code-block:: c

	size_t ARRAY_SIZE(type array[]); /* implemented as a macro */

Returns the number of elements in array. This only works with true arrays
(``type[]``), and will fail to compile when passed a pointer-to-element
(``type *``), which is often used for array access too.


Compile-time build checks
=========================

.. code-block:: c

	int BUILD_BUG_ON_EXPR(bool condition); /* implemented as a macro */
	void BUILD_BUG_ON(bool condition); /* implemented as a macro */

Causes the compiler to fail when condition evaluates to true. If not
implemented for a compiler, it will be a no-op. ``BUILD_BUG_ON`` is meant to be
used as a standalone statement, while ``BUILD_BUG_ON_EXPR`` is for when a check
is to occur within an expression, that latter of which is useful for within
macros when one cannot, or does not want to use multiple statements.

.. code-block:: c

	type DEMOTE_TO_PTR(type expr); /* macro */

Changes the type of ``expr`` to pointer type. If ``expr`` is of array type
class, changes it to a pointer to the first element. If ``expr`` is of function
type class, changes it to a pointer to the function.

.. code-block:: c

	int main(void);
	int (*fp)(void);
	char a[123];
	DEMOTE_TO_PTR(main); /* yields int (*)(void); */
	DEMOTE_TO_PTR(fp);   /* also yields int (*)(void); */
	DEMOTE_TO_PTR(a);    /* yields char * */


UNIX file modes
===============

.. code-block:: c

	#define S_IRUGO   (S_IRUSR | S_IRGRP | S_IROTH)S_IRUGO
	#define S_IWUGO   (S_IWUSR | S_IWGRP | S_IWOTH)S_IWUGO
	#define S_IXUGO   (S_IXUSR | S_IXGRP | S_IXOTH)S_IXUGO
	#define S_IRWXUGO (S_IRUGO | S_IWUGO | S_IXUGO)S_IRWXUGO

The defines make it vastly easier to specify permissions for large group of
users. For example, if one wanted to create a file with the permissions
``rw-r--r--`` (ignoring the umask in this description), ``S_IRUSR | S_IWUSR``
can now be used instead of the longer ``S_IRUSR | S_IWUSR | S_IRGRP |
S_IROTH``.


VC runtime format specifiers
============================

The Microsoft Visual C runtime (a weak libc) uses non-standard format
specifiers for certain types. Whereas C99 specifies ``z`` for ``size_t`` and
``ll`` for ``long long``, MSVCRT users must use ``I`` and ``I64`` (forming
``%Id`` instead of ``%zd`` for ``ssize_t``, for example). libHX provides two
convenience macros for this:

.. code-block:: c

	#define HX_SIZET_FMT    "z" or "I"HX_SIZET_FMT
	#define HX_LONGLONG_FMT "ll" or "I64"HX_LONGLONG_FMT

These may be used together with ``printf`` or ``scanf``:

.. code-block:: c

	printf("struct timespec is of size %" HX_SIZET_FMT "u\n",
	       sizeof(struct timespec));

Take note that mingw-w64's libc *does* adhere to POSIX and so, %z can be used.
