==============
Option parsing
==============

Characteristics:

* short options:
  * bundling of argument-free options (``-a -b -c`` -> ``-abc``)
  * squashing of argument-requiring options (``-a foo`` -> ``-afoo``)
* GNU long options with space or equals sign (``--foo bar``, ``--foo=bar``)
* proper recognition of the lone dash (often used to indicate stdin/stdout)
* recognition of the double dash as option list terminator
* offers POSIX strictness where the option list terminates at the first
  non-option argument
* the parse function is one-shot; there is no state, just a result set
* exclusively uses an option table
* value storing is performed through pointers in the option table
* or user-provided callbacks can be invoked per option


Synopsis
========

.. code-block:: c

	#include <libHX/option.h>

	struct HXoption {
		const char *ln;
		char sh;
		unsigned int type;
		void *ptr, *uptr;
		void (*cb)(const struct HXoptcb *);
		int val;
		const char *help, *htyp;
	};

	int HX_getopt6(const struct HXoption *options_table, int argc, char **argv, struct HXopt6_result *result, unsigned int flags);

The various fields of ``struct HXoption`` are:

``ln``
	The long option name, if any. May be ``NULL`` if none is to be assigned
	for this entry.

``sh``
	The short option name/character, if any. May be '``\0``' if none is to
	be assigned for this entry.

``type``
	The type of the entry, essentially denoting the type of the target
	variable (``ptr``).

``val``
	An integer value to be stored into ``*(int *)ptr`` if the option type
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

Due to the amount of fields, it is advised to use C99/C++20 named initializers
to populate a struct, as they allow to omit unspecified fields, and assume no
specific order of the members:

.. code-block:: c

	struct HXoption e = {.sh = 'f', .help = "Force"};


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

Flags can be combined into the type parameter by OR-ing them. It is valid to
not specify any flags at all, but most flags collide with one another.

``HXOPT_INC``
	Perform an increment on the memory location specified by the
	``*(int *)ptr`` pointer. The referenced variable must be
	initialized.

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

	struct HXopt6_result {
		int nargs;
		const char **uarg;
		char **dup_argv;
	};

	int HX_getopt6(const struct HXoption *options_table, int argc, char **argv, struct HXopt6_result *result, unsigned int flags);
	void HX_getopt6_clean(struct HXopt6_result *);

``HX_getopt6`` is the central parsing function. ``options_table`` specifies the
options that the parser will recognize. ``argv`` must be a vector of C strings,
and ``argc`` be the count of strings that should be processed at most. ``argc``
may be -1, in which case argc is auto-computed from ``argv``, and in this case,
argv must be NULL-terminated.

The ``flags`` argument control the general behavior of ``HX_getopt``:

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
	implicit when the environment variable ``POSIXLY_CORRECT`` is set
	(and ``HXOPT_ANY_ORDER`` is not used).

``HXOPT_ANY_ORDER``
	Specifying this option allows mixing of options and non-options,
	basically the opposite of the strict POSIX order.

``HXOPT_ITER_OPTS``
	``result->desc`` will be filled with pointers to the definitions of the
	parsed options. ``result->oarg`` will be filled with pointers to the
	option argument strings (potentially %nullptr if the option did not
	take anything). ``result->nopts`` will be filled with the option count.

``HXOPT_ITER_ARGS``
	``result->uarg`` will be filled with pointers to leftover arguments
	(pointing into the memory regions of the original argv), and
	``result->nargs`` will contain the string count. uarg does *not*
	contain NULL sentinel, so you cannot iterate with something like ``for
	(const char **p = result.uarg; p != nullptr && *p != nullptr; ++p)``
	but must use ``for (int uidx = 0; uidx < result.nargs; ++uidx)``.

``HXOPT_DUP_ARGS``
	``result->dup_argv`` will be filled with copies of leftover arguments,
	and ``result->nargs`` will contain the string count. dup_argv will
	include the original argv[0]. dup_argv will also include a NULL
	sentinel (not counted in nargs). You can move ``dup_argv`` out of the
	result struct and free it yourself with ``HX_zvecfree`.

The return value of HX_getopt6 can be one of the following:

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

``HXOPT_ERR_FLAGS``
	HX_getopt6 was called with a ``flags`` value that contained illegal or
	silly bit combinations.

negative non-zero
	Failure on behalf of lower-level calls; errno.

Upon HXOPT_ERR_SUCCESS, ``HX_getopt6_clean`` must be called to release
resources.


Pitfalls
========

Staticness of tables
--------------------

The following is an example of a possible pitfall regarding ``HXTYPE_STRDQ``:

.. code-block:: c

	static struct HXdeque *dq;

	int main(int argc, char **argv)
	{
		dq = HXdeque_init();
		static const struct HXoption options_table[] = {
			{.sh = 'N', .type = HXTYPE_STRDQ, .ptr = dq,
			 .help = "Add name"},
			HXOPT_TABLEEND,
		};
		struct HXopt6_result result;
		if (HX_getopt6(options_table, -1, *argv, &result,
		    HXOPT_USAGEONERR) != HXOPT_ERR_SUCCESS)
			return EXIT_FAILURE;
		/* ... */
		HX_getopt6_clean(&result);
		return EXIT_SUCCESS;
	}

The problem here is that ``options_table`` is, due to the static keyword,
initialized at compile-time when ``dq`` is still ``NULL``. To counter this
problem and have it doing the right thing, the ``static`` qualifier on the
options table must be removed, so that the table is built when that line
executes.


Limitations
-----------

The HX option parser has been influenced by both popt and Getopt::Long, but
eventually, there are differences:

* Long options with a single dash (``-foo bar``) are not supported in HXopt.
  This syntax clashes easily with support for option bundling or squashing. In
  case of bundling, ``-foo`` might actually be ``-f -o -o``, or ``-f oo`` in
  case of squashing. It also introduces redundant ways to specify options,
  which is not in the spirit of the author.

* Options using a ``+`` as a prefix, as in ``+foo`` are not supported in HXopt.
  Xterm for example uses it as a way to negate an option. In the author's
  opinion, using one character to specify options is enough — by GNU standards,
  a negator is named ``--no-foo``.

* Table nesting (like in popt) is not supported in HXopt. The need
  has not come up yet. It does however support some forms of chained
  processing, e.g. by using the option terminator, "--".


Examples
========

Storing through pointers
------------------------

.. code-block:: c

	#include <stdio.h>
	#include <stdlib.h>
	#include <libHX/option.h>

	int main(int argc, char **argv)
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

		if (HX_getopt6(options_table, argc, argv, nullptr,
		    HXOPT_USAGEONERR) != HXOPT_ERR_SUCCESS) {
			free(cflag);
			return EXIT_FAILURE;
		}

		printf("aflag = %d, bflag = %d, cvalue = %s\n",
		       aflag, bflag, cflag != NULL ? cflag : "(null)");
		free(cflag);
		return EXIT_SUCCESS;
	}

Note how HXTYPE_STRING in conjunction with ``.ptr=&cflag`` will allocate a
buffer that needs to be freed.

Storing via iteration
---------------------

	#include <stdio.h>
	#include <stdlib.h>
	#include <libHX/option.h>

	int main(int argc, char **argv)
	{
		int aflag = 0;
		int bflag = 0;
		char *cflag = NULL;
		struct HXoption options_table[] = {
			{.sh = 'a', .type = HXTYPE_NONE},
			{.sh = 'b', .type = HXTYPE_NONE},
			{.sh = 'c', .type = HXTYPE_STRING},
			HXOPT_AUTOHELP,
			HXOPT_TABLEEND,
		};

		struct HXopt6_result result;
		if (HX_getopt6(options_table, argc, argv, &result,
		    HXOPT_USAGEONERR | HXOPT_ITER_OPTS) != HXOPT_ERR_SUCCESS)
			return EXIT_FAILURE;
		for (int i = 1; i < result.nopts; ++i) {
			switch (result.desc[i]->sh) {
			case 'a':
				aflag = 1;
				break;
			case 'b':
				bflag = 1;
				break;
			case 'c':
				cflag = result.oarg[i];
				break;
			}
		}
		printf("aflag = %d, bflag = %d, cvalue = %s\n",
		       aflag, bflag, cflag ? cflag : "(null)");
		HX_getopt6_clean(&result);
		return EXIT_SUCCESS;
	}

Note that the pointers in ``oarg`` point to the original argv and so should not
be freed. Upon success of HX_getopt6, HX_getopt6_clean must be called.

Obtaining non-option arguments
------------------------------

.. code-block:: c

	#include <stdio.h>
	#include <stdlib.h>
	#include <libHX/option.h>

	int main(int argc, char **argv)
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

		struct HXopt6_result result;
		if (HX_getopt6(options_table, argc, argv, &result,
		    HXOPT_USAGEONERR | HXOPT_ITER_ARGS) != HXOPT_ERR_SUCCESS) {
			free(cflag);
			return EXIT_FAILURE;
		}
		printf("aflag = %d, bflag = %d, cvalue = %s\n",
		       aflag, bflag, cflag);
		for (int i = 1; i < result.nargs; ++i)
			printf("Non-option argument %s\n", result.uarg[i]);
		free(cflag);
		HX_getopt6_clean(&result);
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

Callbacks
---------

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
