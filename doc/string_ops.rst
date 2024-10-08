=================
String operations
=================

Some string functions are merely present in libHX because they are otherwise
unportable; some are only in the C libraries of the BSDs, some only in GNU
libc.


Locating chars
==============

.. code-block:: c

	#include <libHX/string.h>

	void *HX_memmem(const void *haystack, size_t hsize, const void *needle, size_t nsize);
	char *HX_strbchr(const char *start, const char *now, char delimiter);
	char *HX_strchr2(const char *s, const char *accept);
	size_t HX_strrcspn(const char *s, const char *reject);

``HX_memmem``
	Analogous to ``strstr``(3), ``memmem`` tries to locate the memory block
	pointed to by ``needle`` (which is of length ``nsize``) in the block
	pointed to by ``haystack`` (which is of size ``hsize``). It returns a
	pointer to the first occurrence in ``haystack``, or ``NULL`` when it
	was not found.

``HX_strbchr``
	Searches the character specified by delimiter in the range from ``now``
	to ``start``. It works like ``strrchr``(3), but begins at ``now``
	rather than the ``end`` of the string.

``HX_strchr2``
	This function searches the string ``s`` for any set of bytes that are
	not specified in the second argument, ``n``. In this regard, the
	function is the opposite to ``strpbrk``(3).

``HX_strrcspn``
	Works like ``strcspn``(3), but processes the string from ``end`` to
	``start``.


Extraction
==========

.. code-block:: c

	#include <libHX/string.h>

	char *HX_basename(const char *s);
	char *HX_basename_exact(const char *s);
	char *HX_dirname(const char *s);
	char *HX_strmid(const char *s, long offset, long length);

``HX_basename``
	Returns a pointer to the basename portion of the supplied path ``s``.
	The result of this function is never ``NULL``, and must never be freed
	either. Trailing slashes are not stripped, to avoid having to do an
	allocation. In other words, ``HX_basename("/mnt/")`` will return
	``mnt/``. If you need to have the slashes stripped, use
	``HX_basename_exact``. A possible use for this function is, for
	example, to derive a logging prefix from ``argv[0]``.

.. code-block:: c

	int main(int argc, char **argv)
	{
		if (foo())
			fprintf(stderr, "%s: Special condition occurred.\n",
				HX_basename(argv[0]));
		return 0;
	}

``HX_basename_exact``
	The accurate and safe version of ``HX_basename`` that deals with
	trailing slashes correctly and produces the same result as
	``dirname``(3). It returns a pointer to a newly-allocated string that
	must be freed when done using. ``NULL`` may be returned in case of an
	allocation error.

``HX_dirname``
	Returns a pointer to a new string that contains the directory name
	portion (everything except basename). When done using the string, it
	must be freed to avoid memory leaks.

``HX_strmid``
	Extract a substring of length characters from ``s``, beginning at
	``offset``. If ``offset`` is negative, counting beings from the end of
	the string; -1 is the last character (not the ``'\0'`` byte). If
	``length`` is negative, it will leave out that many characters off the
	end. The function returns a pointer to a new string, and the user has
	to free it.


In-place transformations
========================

.. code-block:: c

	#include <libHX/string.h>

	char *HX_chomp(char *s);
	size_t HX_strltrim(char *s);
	char *HX_stpltrim(const char *s);
	char *HX_strlower(char *s);
	char *HX_strrev(char *s);
	size_t HX_strrtrim(char *s);
	char *HX_strupper(char *s);

``HX_chomp``
	Removes the characters ``'\r'`` and ``'\n'`` from the right edge of the
	string. Returns the original argument.

``HX_strltrim``
	Trims all whitespace (characters on which ``isspace``(3) returns true)
	on the left edge of the string. Returns the number of characters that
	were stripped.

``HX_stpltrim``
	Returns a pointer to the first non-whitespace character in ``s``.

``HX_strlower``
	Transforms all characters in the string ``s`` into lowercase using
	``tolower``(3). Returns the original argument.

``HX_strrev``
	Reverse the string in-place. Returns the original argument.

``HX_strrtrim``
	Trim all whitespace on the right edge of the string. Returns the number
	of characters that were stripped.

``HX_strupper``
	Transforms all characters in the string ``s`` into uppercase using
	``toupper``(3). Returns the original argument.


Out-of-place quoting transforms
===============================

.. code-block:: c

	#include <libHX/string.h>

	char *HX_strquote(const char *s, unsigned int type, char **free_me);

``HX_strquote`` will escape metacharacters in a string according to type, and
returns the escaped result.

Possible values for type:

``HXQUOTE_SQUOTE``
	Escapes all single quotes and backslashes in a string with a backslash.
	(``Ol' \Backslash`` becomes ``Ol\' \\Backslash``.)

``HXQUOTE_DQUOTE``
	Escapes all double quotes and backslahes in a string with the backslash
	method. (``Ol” \Backslash`` becomes ``Ol\” \\Backslash``.)

``HXQUOTE_HTML``
	Escapes ``'<'``, ``'>'``, ``'&'`` and ``'"'`` by their respective HTML
	entities, ``&lt;``, ``&gt;``, ``&amp;`` and ``&quot;``.

``HXQUOTE_LDAPFLT``
	Escapes the string using backslash-plus-hexcode notation as described
	in `RFC 4515`_, to make it suitable for use in an LDAP search
	filter.

``HXQUOTE_LDAPRDN``
	Escapes the string using backslash-plus-hexcode notation as described
	in `RFC 4514`_, to make it suitable for use in an LDAP Relative
	Distinguished Name.

``HXQUOTE_BASE64``
	Transforms the string to BASE64, as described in `RFC 4648`_.

``HXQUOTE_BASE64URL``
	Transforms the string to base64url, as described in `RFC 4648`_.

``HXQUOTE_BASE64IMAP``
	Transforms the string to base64 for IMAP, as described in `RFC 3501`_.

``HXQUOTE_URIENC``
	Escapes the string so that it becomes a valid part for an URI.

``HXQUOTE_SQLSQUOTE``
	Escapes all single quotes in the string by double single-quotes, as
	required for using it in a single-quoted SQL string. No surrounding
	quotes will be generated to facilitate concatenating of HX_strquote
	results.

``HXQUOTE_SQLBQUOTE``
	Escape all backticks in the string by double backticks, as required for
	using it in a backtick-quoted SQL string (used for table names and
	columns). No surrounding ticks will be generated to facilitate
	concatenation.

.. _RFC 4514: http://tools.ietf.org/html/rfc4514
.. _RFC 4515: http://tools.ietf.org/html/rfc4515
.. _RFC 4648: http://tools.ietf.org/html/rfc4648

Specifying an unrecognized type will result in ``NULL`` being returned and
``errno`` be set to ``EINVAL``.

If ``free_me`` is ``NULL``, the function will always allocate memory, even if
the string needs no quoting. The program then has to free the result:

.. code-block:: c

	char *s = HX_strquote("<head>", HXQUOTE_HTML, NULL);
	printf("%s\n", s);
	free(s);

If ``free_me`` is not ``NULL`` however, the function will put the pointer to
the memory area into ``*free_me``, if the string needed quoting. The program
then has to free that after it is done with the quoted result:

.. code-block:: c

	char *tmp = NULL;
	char *s = HX_strquote("head", HXQUOTE_HTML, &tmp);
	printf("%s\n", s);
	free(tmp);

``tmp`` could be ``NULL``, and since ``free(NULL)`` is not an error, this is
perfectly valid. Furthermore, if ``*free_me`` is not ``NULL`` by the time
``HX_strquote`` is called, the function will free it. This makes it possible to
call ``HX_strquote`` in succession without explicit free calls in between:

.. code-block:: c

	char *tmp = NULL;
	printf("%s\n", HX_strquote("<html>", HXQUOTE_HTML, &tmp));
	printf("%s\n", HX_strquote("<head>", HXQUOTE_HTML, &tmp));
	free(tmp);


Tokenizing
==========

.. code-block:: c

	#include <libHX/string.h>libHX/string.h

	char **HX_split(const char *s, const char *delimiters, size_t *fields, int max);
	char **HX_split_inplace(char *s, const char *delimiters, int *fields, int max);
	int HX_split_fixed(char *s, const char *delimiters, int max, char **arr);
	char *HX_strsep(char **sp, const char *delimiters);
	char *HX_strsep2(char **sp, const char *dstr);

``HX_split``
	Splits the string ``s`` on any characters from the ``delimiters``
	string. Both the substrings and the array holding the pointers to these
	substrings will be allocated as required; the original string is not
	modified. If ``max`` is larger than zero, produces no more than ``max``
	fields. If ``fields`` is not ``NULL``, the number of elements produced
	will be stored in ``*fields``. The result is a NULL-terminated array of
	``char *``s, and the user needs to free it when done with it, using
	``HX_zvecfree`` or equivalent. An empty string (zero-length string) for
	``s`` yields a single field.

``HX_split_inplace``
	Splits the string ``s`` in-place on any characters from the
	``delimiters`` string. The resulting array will hold pointers to parts
	of the original string. The array itself needs to be freed by the user,
	using ``free``(3), the individual elements must not be freed. The
	``fields`` and ``max`` arguments work as with ``HX_split``.

``HX_split_fixed``
	Splits the string ``s`` in-place on any characters from the
	``delimiters`` string. The array for the substring pointers must be
	provided by the user through the ``arr`` argument. ``max`` must be the
	number of elements in the array, or less. The array will *not* be
	NULL-terminated[#fixfoot]. The number of fields produced is returned.

.. [#fixfoot] An implementation may however decide to put ``NULL`` in the
              unassigned fields, but this is implementation-dependent.

``HX_strsep``
	Extract tokens from a string. This implementation of strsep has been
	added since the function is non-standard (according to the manpage,
	conforms to BSD4.4 only) and may not be available on every operating
	system. This function extracts tokens, separated by one of the
	characters in ``delimiters``. The string is modified in-place and thus
	must be mutable. The delimiters in the string are then overwritten with
	``'\0'``, ``*sp`` is advanced to the character after the delimiter, and
	the original pointer is returned. After the final token, ``HX_strsep``
	will return ``NULL``.

``HX_strsep2``
	Like ``HX_strsep``, but ``dstr`` is not an array of delimiting
	characters, but an entire substring that acts as one delimiter.


Size-bounded string operations
==============================

.. code-block:: c

	#include <libHX/string.h>

	char *HX_strlcat(char *dest, const char *src, size_t length);
	char *HX_strlcpy(char *dest, const char *src, size_t length);
	char *HX_strlncat(char *dest, const char *src, size_t dlen, size_t slen);
	size_t HX_strnlen(const char *src, size_t max);

``HX_strlcat`` and ``HX_strlcpy`` provide implementations of the
BSD-originating ``strlcat``(3) and ``strlcpy``(3) functions. ``strlcat`` and
``strlcpy`` are less error-prone variants for ``strncat`` and ``strncpy`` as
they always take the length of the entire buffer specified by ``dest``, instead
of just the length that is to be written. The functions guarantee that the
buffer is ``'\0'``-terminated.

``HX_strnlen`` will return the length of the input string or the upper bound
given by ``max``, whichever is less. It will not attempt to access more than
this many bytes in the input buffer.


Allocation-related
==================

.. code-block:: c

	#include <libHX/string.h>

	void *HX_memdup(const void *ptr, size_t length);
	char *HX_strdup(const char *str);
	char *HX_strndup(const char *str, size_t max);
	char *HX_strclone(char **pa, const char *pb);

	#ifdef __cplusplus
	template<typename type> type HX_memdup(const void *ptr, size_t length);
	#endif

``HX_memdup``
	Duplicates `length` bytes from the memory area pointed to by ``ptr``
	and returns a pointer to the new memory block. ``ptr`` may not be
	``NULL``.

``HX_strdup``
	Duplicates the string. The function is equivalent to ``strdup``, but
	the latter may not be available on all platforms. ``str`` may be
	``NULL``, in which case ``NULL`` is also returned.

``HX_strndup``
	Duplicates the input string, but copies at most ``max`` characters.
	(The resulting string will be ``NUL``-terminated of course.) ``str``
	may not be ``NULL``.

``HX_strclone``
	Copies the string pointed to by ``pb`` into ``*pa``. If ``*pa`` was not
	``NULL`` by the time ``HX_strclone`` was called, the string is freed
	before a new one is allocated. The function returns ``NULL`` and sets
	``errno`` to ``EINVAL`` if ``pb`` is ``NULL`` (this way it can be
	freed), or, if ``malloc`` fails, returns ``NULL`` and leaves ``errno``
	at what ``malloc`` had set it to. The use of this function is
	deprecated, albeit no replacement is proposed.


Numbers to human-readable sizes with units
==========================================

.. code-block:: c

	#include <libHX/string.h>

	char *HX_unit_size(char *out, size_t outsize, unsigned long long number,
	                   unsigned int divisor, unsigned int cutoff);
	char *HX_unit_size_cu(char *out, size_t outsize,
	                      unsigned long long number, unsigned int divisor);

``HX_unit_size`` takes an arbitrary number and and produces a more
readily-readable shortened (string) representation with a unit suffix. It does
this by dividing ``number`` by ``pow(divisor, i)`` for some integer _i_ such
that the resulting (integer) quotient is the highest possible value _v_ that is
less than ``cutoff``. This value _v_ is then emitted into ``out`` together with
the corresponding SI prefix.

In other words, ``cutoff`` is the value when it attempts to do another
iteration of the division. For example, if the cutoff is set at 8192,
then 8191 will stay as-is, but 8192 is reduced to "8K". The popular
``wget`` utility implements a cutoff of 1024.

Note that the SI prefix for one iteration (i==1), i.e. kilo, is a lower-case
``'k'``. If you need consistent upper-case output in your program, (i.e. K/M/G
instead of k/M/G), use a subsequent call to ``HX_strupper``.

When ``divisor`` is 0, it defaults to 1000. When ``cutoff`` is 0, an
implementation-defined cutoff point is used. When ``cutoff`` is less than
``divisor``, the result is implementation-defined.

The output number of ``HX_unit_size`` is always integer; no fractions are
emitted. This is rooted in the following idea:

* An output like ``1G`` is quite broad and some precision would be nice. The
  author has historically preferred 3 digits instead of just 2, thanks to wget
  and rsync.

* ``1.34G`` has the same string length as ``1340M``, i.e. both occupy the same
  visual space in console outputs, but the latter has another digit of
  precision.

* By ditching fractions this way, ``HX_unit_size`` also sidesteps the issue of
  excess digits being emitted (usually up to 5) from the trivial use (by
  wget/rsync) of ``printf("%.2f", v)``.

(With regard to the 1.34G-vs-1340M argument, do note that, to actually receive
"``1340M``" as output, you need to set a conveniently high cutoff value such
as 10000. Otherwise, you might get "``1G``".)

The ``HX_unit_size_cu`` function will instead mimic the behavior of coreutils
(/usr/bin/df, /usr/bin/ls). That is, it divides ``number`` by ``pow(divisor,
i)`` for some integer _i_ such that the resulting (real) quotient is
less-than-or-equal ``divisor-1``. It rounds the value up to the next integer if
the fractional part is >90%, and if the quotient is greater-or-equal 10, the
fractional part is stripped and not emitted to ``out``.

In practice, the rounding up of ``HX_unit_size_cu`` lends itself to display
occupying sizes, whereas the implicit rounding down (of integer divisions)
in ``HX_unit_size`` lend itself to sizes in progress meters.


Unit-suffixed numbers to full numbers
=====================================

.. code-block:: c

	#include <libHX/string.h>

	double HX_strtod_unit(const char *s, char **end,
	                      unsigned int exponent);
	unsigned long long HX_strtoull_unit(const char *s, char **end,
	                                    unsigned int exponent);

The ``HX_strtod_unit`` and ``HX_strtoull_unit`` functions behave similar to
``strtod`` and ``strtoul``, respectively, in that they convert the initial part
of the string in ``s`` to a ``double`` and ``unsigned long long``,
respectively, and apply the selected multiplication factor from ``exponent`` in
resolving an optional unit suffix.

Upon overflow, ``errno`` is set to ``ERANGE`` just like the stdlib functions.
Unlike some implementations of ``strtoul``, negative numbers are outright
rejected.

.. code-block:: c

	unsigned long long bytes = HX_strtoull_unit("1.5G", NULL, 1024);


Conversion from/to human-readable durations with units
======================================================

.. code-block:: c

	#include <libHX/string.h>

	unsigned long long HX_strtoull_sec(const char *s, char **end);
	unsigned long long HX_strtoull_nsec(const char *s, char **end);
	char *HX_unit_seconds(char *out, size_t outsize,
	                      unsigned long long seconds,
	                      unsigned int flags);

``HX_strtoull_sec`` and ``HX_strtoull_nsec`` convert a time duration with
units, such as ``"15min30s"`` into an all-seconds and all-nanoseconds value,
respectively. The recognized unit strings are: ``years``, ``year``, ``y``,
``months``, ``month``, ``days``, ``day``, ``d``, ``hours``, ``hour``, ``h``,
``minutes``, ``minute``, ``min``, ``seconds``, ``second``, ``s``, the empty
string (to mean seconds), ``msec``, ``ms``, ``µsec``, ``µs``, ``nsec`` and
``ns``. Negative durations are not supported. Support for fractions is
implementation-defined. When parsing stops at any point, ``*end`` is set to the
location, similar to how the ``strtoull`` C function would.

One year is defined to be 365.25 days of 86400 seconds; one month is defined to
be 1/12 such a year. This is consistent with the units employed by systemd.

``HX_unit_seconds`` is the reverse and transforms the duration given by
``seconds`` into a string representation broken into days, hours, minutes, and
remaining seconds as appropriate. By default, only the d/h/min/s units are
emitted. The ``flags`` argument specifies if any other units should be emitted;
``HXUNIT_YEARS``, ``HXUNIT_MONTHS`` and ``HXUNIT_WEEKS`` are available. The
longest string HX_unit_seconds can emit on a contemporary 64-bit POSIX system
with 1970 epoch is 40 characters (so a buffer should be no less than 41 bytes),
cf. ``HXSIZEOF_UNITSEC64``.


Examples
========

Using HX_split_fixed
--------------------

``HX_split_fixed`` is often used just with scoped automatic-storage variables
and where the field count of interest is fixed, as the example for parsing
``/etc/passwd`` shows:

.. code-block:: c

	#include <stdio.h>
	#include <libHX/string.h>

	char *field[8];
	hxmc_t *line = NULL;

	while (HX_getl(&line, fp) != NULL) {
		if (HX_split_fixed(line, ":", ARRAY_SIZE(field), field) < 7) {
			fprintf(stderr, "That does not look like a valid line.\n");
			continue;
		}
		printf("Username: %s\n", field[0]);
	}

Using HX_split_inplace
----------------------

Where the number of fields is not previously known and/or estimatable, but the
string can be modified in place, one uses ``HX_split_inplace`` as follows:

.. code-block:: c

	#include <errno.h>
	#include <stdio.h>
	#include <libHX/string.h>

	while (HX_getl(&line, fp) != NULL) {
		char **field = HX_split_inplace(line, ":", NULL, 0);
		if (field == NULL) {
			fprintf(stderr, "Badness! %s\n", strerror(errno));
			break;
		}
		printf("Username: %s\n", field[0]);
		free(field);
	}

Using HX_split
--------------

Where the string is not modifiable in-place, one has to resort to using the
full-fledged ``HX_split`` that allocates space for each substring.

.. code-block:: c

	#include <errno.h>
	#include <stdio.h>
	#include <libHX/string.h>

	while (HX_getl(&line, fp) != NULL) {
		char **field = HX_split(line, ":", NULL, 0);
		if (field == NULL) {
			fprintf(stderr, "Badness. %s\n", strerror(errno));
			break;
		}
		printf("Username: %s\n", field[0]);
		/* Suppose “callme” needs the original string */
		callme(line);
		HX_zvecfree(field);
	}

Using HX_strsep
---------------

``HX_strsep`` provides for thread- and reentrant-safe tokenizing a string where
strtok from the C standard would otherwise fail.

.. code-block:: c

	#include <stdio.h>
	#include <libHX/string.h>

	char line[] = "root:x:0:0:root:/root:/bin/bash";
	char *wp, *p;

	wp = line;
	while ((p = HX_strsep(&wp, ":")) != NULL)
		printf("%s\n", p)
