=======================
String format templates
=======================

HXfmt is a template system for by-name variable expansion. It can be used to
substitute placeholders in format strings supplied by the user by appropriate
expanded values defined by the program. Such can be used to allow for flexible
configuration files that define key-value mappings such as

::

	detect_peer = ping6 -c1 %(ADDR)
	#detect_peer = nmap -sP %(ADDR) | grep -Eq "appears to be up"

Consider, for example, a monitoring daemon that allows the administrator to
specify a program of his choice with which to detect whether a peer is alive or
not. The user can choose any program that is desired, but evidently needs to
pass the address to be tested to the program. This is where the daemon will do
a substitution of the string ``ping -c1 %(ADDR)`` it read from the config file,
and put the actual address in it before finally executing the command.

.. code-block:: c

	printf("%s has %u files\n", user, num);
	printf("%2$u files belong to %1$s\n", num, user);

``%s`` (or ``%1$s`` here) specifies how large ``user`` is — ``sizeof(const char
*)`` in this case. If that is missing, there is no way to know the offset of
``num`` relative to ``user``, making varargs retrieval impossible.

``printf``, at least from GNU libc, has something vaguely similar: positional
parameters. They have inherent drawbacks, though. One is of course the question
of portability, but there is a bigger issue. All parameters must be specified,
otherwise there is no way to determine the location of all following objects
following the missing one on the stack in a varargs-function like printf, which
makes it unsuitable to be used with templates where omitting some placeholders
is allowed.

Initialization, use and deallocation
====================================

.. code-block:: c

	#include <libHX/option.h>

	struct HXformat_map *HXformat_init(void);
	void HXformat_free(struct HXformat_map *table);
	int HXformat_add(struct HXformat_map *table, const char *key, const void *ptr, unsigned int ptr_type);

``HXformat_init`` will allocate and set up a string-to-string map that is used
for the underlying storage, and returns it.

To release the substitution table and memory associated with it, call
``HXformat_free``.

``HXformat_add`` is used to add substitution entries. One can also specify
other types such as numeric types. ``ptr_type`` describes the type behind
``ptr`` and the constants are the same from ``option.h`` (cf. section on
optionp arsing) — not all constants can be used, though, and their meaning also
differs from what ``HX_getopt`` or ``HX_shconfig`` use them for — the two could
be seen as “read” operations, while ``HXformat`` is a write operation.

Immediate types
===============

“Immediate types” are resolved when ``HXformat_add`` is called, that is, they
are copied and inserted into the tree, and are subsequently independent from
any changes to variables in the program. Because the HXopt-originating type
name, i.e. ``HXTYPE_*``, is also used for deferred types, the constant
``HXFORMAT_IMMED`` needs to be specified on some types to denote an immediate
value.

* ``HXTYPE_STRING`` — ptr is a ``const char *``.

* ``HXTYPE_{U,}{CHAR,SHORT,INT,LONG,LLONG} | HXFORMAT_IMMED`` —
  mapping to the standard typesk

Deferred types
==============

“Deferred types” are resolved on every invocation of a formatter function
(``HXformat_*printf``). The expansions may be changed by modifying the
underlying variable pointed to, but the pointer must remain valid and its
pointee not go out of scope. Code samples are provided below.

* ``HXTYPE_STRP`` — ptr is a ``const char *const *``; the
  pointer resolution is deferred until the formatter is called with one of the
  ``HXformat_*printf`` functions. Deferred in the sense it is always resolved
  anew.

* ``HXTYPE_BOOL`` — ptr is a ``const int *``.

* ``HXTYPE_{U,}..``, ``HXTYPE_FLOAT``, ``HXTYPE_DOUBLE`` — mapping to the
  standard types with one indirection (e.g. ``int *``).

Invoking the formatter
======================

.. code-block:: c

	int HXformat_aprintf(struct HXformat_map *table, hxmc_t **dest, const char *template);
	int HXformat_sprintf(struct HXformat_map *table, char *dest, size_t size, const char *template);
	int HXformat_fprintf(struct HXformat_map *table, FILE *filp, const char *template);

``HXformat_aprintf``
	Substitutes placeholders in template using the given table. This will
	produce a string in a HX memory container (``hxmc_t``), and the pointer
	is placed into ``*dest``. The caller will be responsible for freeing it
	later when it is done using the result.

``HXformat_sprintf``
	Does substitution and stores the expanded result in the buffer ``dest``
	which is of size ``size``.

``HXformat_fprintf``
	Does substituion and directly outputs the expansion to the given stdio
	stream.

On success, the length of the expanded string is returned (only up to a maximum
of SSIZE_MAX), excluding the trailing ``\0``. While ``HXformat_sprintf`` will
not write more than ``size`` bytes (including the ``\0``), the length it would
have taken is returned, similar to what sprintf does. On error, ``-errno`` is
returned.

The HXformat function family recognizes make-style like functions and recursive
expansion, described below.

Functions
=========

To expand a variable, one uses a syntax like ``%(NAME)`` in the format string.
Recursive expansion like ``%(%(USER))`` is supported; assuming ``%(USER)``
would expand to ``linux``, HXformat would try to resolve ``%(linux)`` next.
Besides these variable substitutions, HXformat also provides function calls
whose syntax isx ``%(nameOfFunction parameters[...])``. Parameters can be any
text, including variables. Paramters are separated from another by a delimiter
specific to each function. See this list for details:

* ``%(env variable)``

  The ``env`` function expands to the string that is stored in the
  environmental variable by the given name.

* ``%(exec command [args...])``

  The ``exec`` function expands to the standard output of the command. The
  command is directly run without shell invocation, so no special character
  expansion (wildcards, etc.) takes place. stdin is set to ``/dev/null``. The
  parameter delimiter is the space character. To be able to use this function —
  as it is relevant to security — the fmt table needs to have a key
  with the magic value ``/libhx/exec``.

* ``%(if condition,[then][,[else]])``

  If the condition parameter expands to a string of non-zero length, the
  function expands to the ``then`` block, otherwise the ``else`` block. The
  delimiter used is a comma.

* ``%(lower text)``, ``%(upper text)``

  Lowercases or uppercases the supplied argument. As these functions are meant
  to take only one argument, there is no delimiter defined that would need
  escaping if multiple arguments were supposed to be passed. ``%(lower a,b)``
  is equivalent to ``%(lower "a,b")``.

* ``%(shell command [args...])``

  Similar to ``%(exec)``, but invokes the shell inbetween (i.e. ``sh -c
  'command...'``) such that special characters, redirection, and so on can be
  used.

* ``%(substr text,offset[,length])``

  Extracts a substring out of the given text, starting at offset and running
  for the given length. If no length is given, will extract until the end of
  the string. If ``offset`` is negative, it specifies the offset from the end
  of the string. If ``length`` is negative, that many characters are left off
  the end.

* ``%(snl text)``

  Strips trailing newlines from text and replaces any other newline by a space.
  What happens implicity in Makefiles' ``$(shell ...)`` statements usually is
  explicitly separate in libHX.

Example: Immediate and deferred resolution
==========================================

.. code-block:: c

	const char *b = "Hello World";
	char c[] = "Hello World";
	struct HXformat_map *table = HXformat_init();
	HXformat_add(table, "%(GREETING1)", b, HXTYPE_STRING);
	HXformat_add(table, "%(GREETING2)", &c, HXTYPE_STRP);
	b = NULL;
	snprintf(c, sizeof(c), "Hello Home");
	HXformat_aprintf(...);

Upon calling ``HXformat_*printf``, ``%(GREETING1)`` will expand to ``Hello
World`` whereas ``%(GREETING2)`` will expand to ``Hello Home``.


Example: Using the %(exec) function
===================================

.. code-block:: c

	struct HXformat_map *table = HXformat_init();
	HXformat_add(table, "/libhx/exec", NULL, HXTYPE_IMMED);
	HXformat_aprintf(table, &result, "%(exec uname -s)");
