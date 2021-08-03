==============
Option parsing
==============

libHX uses a table-based approach like libpopt[#f3]. It provides for both long
and short options and the different styles associated with them, such as
absence or presence of an equals sign for long options (``--foo=bar`` and ``--foo
bar``), bundling (writing ``-abc`` for non-argument taking options ``-a -b -c``),
squashing (writing ``-fbar`` for an argument-requiring option ``-f bar``). The “lone
dash” that is often used to indicate standard input or standard output, is
correctly handled[#f4], as in ``-f -``.

.. [#f3] The alternative would be an iterative, open-coded approach like
         ``getopt``(3) requires.

.. [#f4] popt failed to do this for a long time.

A table-based approach allows for the parser to run as one atomic block of code
(callbacks are, by definition, “special” exceptions), making it more opaque
than an open-coded ``getopt``(3) loop. You give it your argument vector and the
table, snip the finger (call the parser function once), and it is done. In
getopt on the other hand, the getopt function returns for every argument it
parsed and needs to be called repeatedly.


Synopsis
========

.. code-block:: c

	#include <libHX/option.h>

	struct HXoption {struct HXoption
		const char *ln;
		char sh;
		unsigned int type;
		void *ptr, *uptr;
		void (*cb)(const struct HXoptcb *);
		int val;
		const char *help, *htyp;
	};

	int HX_getopt(const struct HXoption *options_table, int *argc, const char ***argv, unsigned int flags);

The various fields of ``struct HXoption`` are:

``ln``
	The long option name, if any. May be ``NULL`` if none is to be assigned
	for this entry.

``sh``
	The short option name/character, if any. May be '``\0``' if none is to
	be assigned for this entry.

``type``
	The type of the entry, essentially denoting the type of the target
	variable.

``val``
	An integer value to be stored into ``*(int *)ptr`` when the option type
	is ``HXTYPE_IVAL``.

``ptr``
	A pointer to the variable so that the option parser can store the
	requested data in it. The pointer may be ``NULL``, in which case no
	data is stored (but ``cb`` is still called if defined, with the data).

``uptr``
	A user-supplied pointer. Its value is passed verbatim to the callback,
	and may be used for any purpose the user wishes. If the option type is
	``HXTYPE_SVAL``, it is the value in uptr that will be used to populate
	``*(const char **)ptr``.

``cb``
	If not ``NULL``, call out to the referenced function after the option
	has been parsed (and the results possibly be stored in ``ptr``).

``help``
	A help string that is shown for the option when the option table is
	dumped by request (e.g. ``yourprgram --help``).

``htyp``
	String containing a keyword to aid the user in understanding the
	available options during dump. See examples.

Due to the amount of fields, it is advised to use C99 named initializers to
populate a struct, as they allow to omit unspecified fields, and assume no
specific order of the members:

.. code-block:: c

	struct HXoption e = {.sh = 'f', .help = "Force"};

It is a sad fact that C++ has not gotten around to implement named initializers
as of C++17. It is possible to put the option parsing code into a separate C
source file that can then be compiled in C99 rather than C++ mode.


Type map
========

``HXTYPE_NONE``
	The option does not take any argument, but the presence of the option
	may be record by setting ``*(int *)ptr`` to ``1``. Other rules apply
	when ``HXOPT_INC`` or ``HXOPT_DEC`` are specified as flags.

``HXTYPE_VAL``
	Use the integer value specified by ``ival`` and store it in
	``*(int *)ptr``.

``HXTYPE_SVAL``
	Use the memory location specified by ``sval`` and store it in ``*(const
	char **)ptr``. OUTDATED.

``HXTYPE_BOOL``
	Interpret the supplied argument as a boolean descriptive (must be
	``yes``, ``no``, ``on``, ``off``, ``true``, ``false``, ``0`` or ``1``)
	and store the result in ``*(int *)ptr``.

``HXTYPE_STRING``
	The argument string is duplicated to a new memory region and the
	resulting pointer stored into ``*(char **)ptr``. This incurs an
	allocation so that subsequently modifying the original argument string
	in any way will not falsely propagate.

``HXTYPE_STRDQ``
	The argument string is duplicated to a new memory region and the
	resulting pointer is added to the given HXdeque. Note that you often
	need to use deferred initialization of the options table to avoid
	putting ``NULL`` into the entry. See section about pitfalls.

The following overview lists the types that map to the common integral and
floating-point types. Signed and unsigned integeral types are processed using
``strtol`` and ``strtoul``, respectively. ``strtol`` and ``strtoul`` will be
called with automatic base detection. This usually means that a leading ``0``
indicates the string is given in octal base, a leading ``0x`` indicates
hexadecimal base, and decimal otherwise. ``HXTYPE_LLONG``, ``HXTYPE_ULLONG``,
``HXTYPE_INT64`` and ``HXTYPE_UINT64`` use ``strtoll`` and/or ``strtoull``,
which may not be available on all platforms.

``HXTYPE_CHAR``
	maps to ``char``

``HXTYPE_UCHAR``
	maps to ``unsigned char``

``HXTYPE_SHORT``
	maps to ``short``

``HXTYPE_USHORT``
	maps to ``unsigned short``

``HXTYPE_INT``
	maps to ``int``

``HXTYPE_UINT``
	maps to ``unsigned int``

``HXTYPE_LONG``
	maps to ``long``

``HXTYPE_ULONG``
	maps to ``unsigned long``

``HXTYPE_LLONG``
	maps to ``long long``

``HXTYPE_ULLONG``
	maps to ``unsigned long long``

``HXTYPE_SIZE_T``
	maps to ``size_t``

``HXTYPE_FLOAT``
	maps to ``float``

``HXTYPE_DOUBLE``
	maps to ``double``

``HXTYPE_INT8``
	maps to ``int8_t``

``HXTYPE_UINT8``
	maps to ``uint8_t``

``HXTYPE_INT16``
	maps to ``int16_t``

``HXTYPE_UINT16``
	maps to ``uint16_t``

``HXTYPE_INT32``
	maps to ``int32_t``

``HXTYPE_UINT32``
	maps to ``uint32_t``

``HXTYPE_INT64``
	maps to ``int64_t``

``HXTYPE_UINT64``
	maps to ``uint64_t``

``HXTYPE_FLOAT`` and ``HXTYPE_DOUBLE`` make use of ``strtod`` (``strtof`` is
not used). A corresponding type for the ``long double`` format is not
specified, but may be implemented on behalf of the user via a callback.


Flags
=====

Flags can be combined into the type parameter by OR'ing them. It is valid to
not specify any flags at all, but most flags collide with one another.

``HXOPT_INC``
	Perform an increment on the memory location specified by the
	``*(int *)ptr`` pointer. Make sure the referenced variable is
	initialized beforehand!

``HXOPT_DEC``
	Perform a decrement on the pointee. Same requirements as ``HXOPT_INC``.

Only one of ``HXOPT_INC`` and ``HXOPT_DEC`` may be specified at a time,
and they require that the base type is ``HXTYPE_NONE``, or they will
have no effect. An example may be found below.

``HXOPT_NOT``
	Binary negation of the argument directly after reading it from the
	command line into memory. Any of the three following operations are
	executed with the already-negated value.

``HXOPT_OR``
	Apply bitwise OR on the pointee with the specified/transformed value.

``HXOPT_AND``
	Apply bitwise AND on the pointee with the specified/transformed value.

``HXOPT_XOR``
	Apply bitwise XOR on the pointee with the specified/transformed value.

Only one of ``HXOPT_OR``, ``HXOPT_AND`` and ``HXOPT_XOR`` may be specified at
a time, but they can be used with any integral type (``HXTYPE_UINT``,
``HXTYPE_ULONG``, etc.). An example can be found below.

``HXOPT_OPTIONAL``
	This flag allows for an option to take zero or one argument. Needless
	to say that this can be confusing to the user. iptables's ``-L`` option
	for example is one of this kind (though it does not use the libHX
	option parser). When this flag is used, ``-f -b`` is interpreted as
	``-f`` without an argument, as is ``-f --bar`` — things that look like
	an option take precedence over an option with an optional argument.
	``-f -`` of course denotes an option with an argument, as ``-`` is
	often used to indicate standard input/output.


Special entries
===============

HXopt provides two special entries via macros:

``HXOPT_AUTOHELP``
	Adds entries to recognize ``-?`` and ``--help`` that will display the
	(long-format) help screen, and ``--usage`` that will display the short
	option syntax overview. All three options will exit the program
	afterwards.

``HXOPT_TABLEEND``
	This sentinel marks the end of the table and is required on all tables.
	(See examples for details.)


Invoking the parser
===================

.. code-block:: c

	int HX_getopt(const struct HXoption *options_table, int *argc, const char ***argv, unsigned int flags);HX_getopt

``HX_getopt`` is the actual parsing function. It takes the option table, and a
pointer to your argc and argv variables that you get from the main function.
The parser will, unlike GNU getopt, literally “eat” all options and their
arguments, leaving only non-options in ``argv``, and ``argc`` updated, when
finished. This is similar to how Perl's ``Getopt::Long`` module works.
Additional flags can control the exact behavior of ``HX_getopt``:

``HXOPT_PTHRU``
	“Passthrough mode”. Any unknown options are not “eaten” and are instead
	passed back into the resulting argv array.

``HXOPT_QUIET``
	Do not print any diagnostics when encountering errors in the user's
	input.

``HXOPT_HELPONERR``
	Display the (long-format) help when an error, such as an unknown option
	or a violation of syntax, is encountered.

``HXOPT_USAGEONERR``
	Display the short-format usage syntax when an error is encountered.

``HXOPT_RQ_ORDER``
	Specifying this option terminates option processing when the first
	non-option argument in argv is encountered. This behavior is also
	implicit when the environment variable ``POSIXLY_CORRECT`` is set.

The return value can be one of the following:

``HXOPT_ERR_SUCCESS``
	Parsing was successful.

``HXOPT_ERR_UNKN``
	An unknown option was encountered.

``HXOPT_ERR_VOID``
	An argument was given for an option which does not allow one. In
	practice this only happens with ``--foo=bar`` when ``--foo`` is of type
	``HXTYPE_NONE``, ``HXTYPE_VAL`` or ``HXTYPE_SVAL``. This does not
	affect ``--foo bar``, because this can be unambiguously interpreted as
	``bar`` being a remaining argument to the program.

``HXOPT_ERR_MIS``
	Missing argument for an option that requires one.

``HXOPT_ERR_AMBIG``
	An abbreviation of a long option was ambiguous.

negative non-zero
	Failure on behalf of lower-level calls; errno.


Pitfalls
========

Staticness of tables
--------------------

The following is an example of a possible pitfall regarding ``HXTYPE_STRDQ``:

.. code-block:: c

	static struct HXdeque *dq;

	static bool get_options(int *argc, const char ***argv)
	{
		static const struct HXoption options_table[] = {
			{.sh = 'N', .type = HXTYPE_STRDQ, .ptr = dq,
			 .help = "Add name"},
			HXOPT_TABLEEND,
		};
		return HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) ==
		       HXOPT_ERR_SUCCESS;
	}

	int main(int argc, const char **argv)
	{
		dq = HXdeque_init();
		get_options(&argc, &argv);
		return 0;
	}

The problem here is that ``options_table`` is, due to the static keyword,
initialized at compile-time when ``dq`` is still ``NULL``. To counter this
problem and have it doing the right thing, you must remove the static qualifier
on the options table when used with ``HXTYPE_STRDQ``, so that it will be
evaluated when it is first executed.

It was not deemed worthwhile to have ``HXTYPE_STRDQ`` take an indirect
``HXdeque`` (``struct HXdeque **``) instead just to bypass this issue. (Live
with it.)

Limitations
-----------

The HX option parser has been influenced by both popt and Getopt::Long, but
eventually, there are differences:

* Long options with a single dash (``-foo bar``). This unsupported
  syntax clashes easily with support for option bundling or squashing. In case
  of bundling, ``-foo`` might actually be ``-f -o -o``, or ``-f oo`` in case of
  squashing. It also introduces redundant ways to specify options, which is not
  in the spirit of the author.

* Options using a ``+`` as a prefix, as in ``+foo``. Xterm for
  example uses it as a way to negate an option. In the author's opinion, using
  one character to specify options is enough — by GNU standards, a negator is
  named ``--no-foo``.

* Table nesting like implemented in popt. HXopt has no provision for nested
  tables, as the need has not come up yet. It does however support chained
  processing. You cannot do nested tables even with callbacks, as the new argv
  array is only put in place shortly before ``HX_getopt`` returns.


Examples
========

Basic example
-------------

The following code snippet should provide an equivalent of the
GNU getopt sample.[#f5]

.. [#f5] http://www.gnu.org/software/libtool/manual/libc/Example-of-Getopt.html\#Example-of-Getopt

.. code-block:: c

	#include <stdio.h>
	#include <stdilb.h>
	#include <libHX/option.h>

	int main(int argc, const char **argv)
	{
		int aflag = 0;
		int bflag = 0;
		char *cflag = NULL;

		struct HXoption options_table[] = {
			{.sh = 'a', .type = HXTYPE_NONE, .ptr = &aflag},
			{.sh = 'b', .type = HXTYPE_NONE, .ptr = &bflag},
			{.sh = 'c', .type = HXTYPE_STRING, .ptr = &cflag},
			HXOPT_AUTOHELP,
			HXOPT_TABLEEND,
		};

		if (HX_getopt(options_table, &argc, &argv, HXOPT_USAGEONERR) !=
		    HXOPT_ERR_SUCCESS)

			return EXIT_FAILURE;

		printf("aflag = %d, bflag = %d, cvalue = %s\n",
		       aflag, bflag, cvalue);

		while (*++argv != NULL)
			printf("Non-option argument %s\n", *argv);

		return EXIT_SUCCESS;
	}

Verbosity levels
----------------

.. code-block:: c

	static int verbosity = 1; /* somewhat silent by default */
	static const struct HXoption options_table[] = {
		{.sh = 'q', .type = HXTYPE_NONE | HXOPT_DEC, .ptr = &verbosity,
		 .help = "Reduce verbosity"},
		{.sh = 'v', .type = HXTYPE_NONE | HXOPT_INC, .ptr = &verbosity,
		 .help = "Increase verbosity"},
		HXOPT_TABLEEND,
	};

This sample option table makes it possible to turn the verbosity of the program
up or down, depending on whether the user specified ``-q`` or ``-v``. By passing
multiple ``-v`` flags, the verbosity can be turned up even more. The range depends
on the ``int`` data type for your particular platform and compiler; if you want
to have the verbosity capped at a specific level, you will need to use an extra
callback:

.. code-block:: c

	static int verbosity = 1;

	static void v_check(const struct HXoptcb *cbi)
	{
		if (verbosity < 0)
			verbosity = 0;
		else if (verbosity > 4)
			verbosity = 4;
	}

	static const struct HXoption options_table[] = {
		{.sh = 'q', .type = HXTYPE_NONE | HXOPT_DEC, .ptr = &verbosity,
		 .cb = v_check, .help = "Lower verbosity"},
		{.sh = 'v', .type = HXTYPE_NONE | HXOPT_INC, .ptr = &verbosity,
		 .cb = v_check, .help = "Raise verbosity"},
		HXOPT_TABLEEND,
	};

Mask operations
---------------

.. code-block:: c

	/* run on all CPU cores by default */
	static unsigned int cpu_mask = ~0U;
	/* use no network connections by default */
	static unsigned int net_mask = 0;
	static struct HXoption options_table[] = {
		{.sh = 'c', .type = HXTYPE_UINT | HXOPT_NOT | HXOPT_AND, .ptr = &cpu_mask,
		 .help = "Mask of cores to exclude", .htyp = "cpu_mask"},
		{.sh = 'n', .type = HXTYPE_UINT | HXOPT_OR, .ptr = &net_mask,

		 .help = "Mask of network channels to additionally use",
		 .htyp = "channel_mask"},
		HXOPT_TABLEEND,
	};

What this options table does is ``cpu_mask &= ~x`` and ``net_mask |= y``, the
classic operations of clearing and setting bits.

Support for non-standard actions
--------------------------------

Supporting additional types or custom storage formats is easy, by simply using
``HXTYPE_STRING``, ``NULL`` as the data pointer (usually by not specifying it
at all), the pointer to your data in the user-specified pointer ``uptr``, and
the callback function in ``cb``.

.. code-block:: c

	struct fixed_point {
		int integral;
		unsigned int fraction;
	};

	static struct fixed_point number;

	static void fixed_point_parse(const struct HXoptcb *cbi)
	{
		char *end;

		number.integral = strtol(cbi->data, &end, 0);
		if (*end == '\0')
			number.fraction = 0;
		else if (*end == '.')
			number.fraction = strtoul(end + 1, NULL, 0);
		else
			fprintf(stderr, "Illegal input.\n");
	}

	static const struct HXoption options_table[] = {
		{.sh = 'n', .type = HXTYPE_STRING, .cb = fixed_point_parse,
		 .uptr = &number, .help = "Do this or that",
		HXOPT_TABLEEND,

	};

Chained argument processing
---------------------------

On the first run, only ``--cake`` and ``--fruit`` is considered, which is then
used to select the next set of accepted options. Note that
``HXOPT_DESTROY_OLD`` is used here, which causes the ``argv`` that is produced
by the first invocation of ``HX_getopt`` in the ``get_options`` function to be
freed as it gets replaced by a new argv again by ``HX_getopt`` in
``get_cakes``/``get_fruit``. ``HXOPT_DESTROY_OLD`` is however not specified in
the first invocation, because the initial argv resides on the stack and cannot
be freed.

.. code-block:: c

	static bool get_cakes(int *argc, const char ***argv)
	{
		struct HXoption option_table[] = {
			...
		};
		return HX_getopt(cake_table, argc, argv,
		       HXOPT_USAGEONERR | HXOPT_DESTROY_OLD) == HXOPT_ERR_SUCCESS;
	}

	static bool get_fruit(int *argc, const char ***argv)
	{
		struct HXoption fruit_table[] = {
			...
		};
		return HX_getopt(fruit_table, argc, argv,
		       HXOPT_USAGEONERR | HXOPT_DESTROY_OLD) == HXOPT_ERR_SUCCESS;
	}

	static bool get_options(int *argc, const char ***argv)
	{
		int cake = 0, fruit = 0;
		struct HXoption option_table[] = {
			{.ln = "cake",  .type = HXTYPE_NONE, .ptr = &cake},
			{.ln = "fruit", .type = HXTYPE_NONE, .ptr = &fruit},
			HXOPT_TABLEEND,
		};
		if (HX_getopt(option_table, argc, argv, HXOPT_PTHRU) != HXOPT_ERR_SUCCESS)
			return false;
		if (cake)
			return get_cakes(argc, argv);
		else if (fruit)
			return get_fruit(argc, argv);
		return false;
	}
