=====================================
Shell-style configuration file parser
=====================================

libHX provides functions to read shell-style configuration files. Such files
are common, for example, in ``/etc/sysconfig`` on Linux systems. The format is
pretty basic; it only knows about ``key=value`` pairs and does not even have
sections like INI files. Not relying on any features however makes them quite
interchangable as the syntax is accepted by Unix Shells.

Lines beginning with a hash mark (``#``) are ignored, as are empty lines and
unrecognized keys.

.. code-block:: sh

	# Minimum / maximum values for automatic UID selection
	UID_MIN=100
	UID_MAX=65000

	# Home directory base
	HOME="/home"
	#HOME="/export/home"

Any form of variable or parameter substitution or expansion is highly
implementation specific, and is not supported in libHX's reader. Even Shell
users should not rely on it as you never know in which context the
configuration files are evaluated. Still, you will have to escape specific
sequences like you would need to in Shell. The use of single quotes is
acceptable. That means::

.. code-block:: sh

	AMOUNT="US\$5"
	AMOUNT='US$5'

Synopsis
========

.. code-block:: c

	#include <libHX/option.h>

	int HX_shconfig(const char *file, const struct HXoption *table);
	int HX_shconfig_pv(const char **path_vec, const char *file, const struct HXoption *table, unsigned int flags);
	struct HXmap *HX_shconfig_map(const char *file);

The shconfig parser reuses ``struct HXoption`` that fits very well in
specifying name-pointer associations. ``HX_shconfig`` will read the given file
using the key-to-pointer mappings from the table to store the variable
contents. Of ``struct HXoption``, only the ``ln``, ``type`` and ``ptr`` fields
are used. The list of accepted types is described in
section [subsec:option-types].

To parse a file, call ``HX_shconfig`` function with the corresponding
parameters. If you want to read configuration files from different paths, i.e.
to build up on default values, you can use ``HX_shconfig_pv``, which is a
variant for reading a file from multiple locations. Its purpose is to
facilitate reading system-wide settings which are then overriden by a file in
the users home directory, for example (per-setting-override). It is also
possible to do per-file-override, that is, a file in the home directory has
higher precedence than a system-wide one in such a way that the system-wide
configuration file is not even read. This is accomplished by traversing the
paths in the “other” direction (actually you have to turn the array around) and
stopping at the first existing file by use of the ``SHCONF_ONE`` flag.

.. [#f2] pv = path vector

``HX_shconfig_map`` will return all entries from the file in a HXmap, usable
for parsing arbitrary keys without having to specify any static key table.

``SHCONF_ONE``
	Parsing files will stop after one file has been successfully parsed.
	This allows for a “personal overrides system config” style.

The call to ``HX_shconfig`` will either return >0 for success, 0 for no success
(actually, this is never returned) and ``-errno`` for an error.

Example
=======

Per-setting-override
--------------------

This example sources key-value pairs from a configuration file in a system
location (``/etc``) first, before overriding specific keys with new values from the
file in the home directory.

.. code-block:: c

	long uid_min, uid_max;
	char *passwd_file;
	struct HXoption options_table[] = {
		{.ln = "UID_MIN",  .type = HXTYPE_LONG,   .ptr = &uid_min},
		{.ln = "UID_MAX",  .type = HXTYPE_LONG,   .ptr = &uid_max},
		{.ln = "PWD_FILE", .type = HXTYPE_STRING, .ptr = &passwd_file},
		HXOPT_TABLEEND,
	};
	const char *home = getenv("HOME");
	const char *paths[] = {"/etc", home, NULL};
	HX_shconfig(paths, "test.cf", options_table, 0);

Per-file-override
-----------------

This particular example reads from the file in the home directory first (if it
exists), but stops after it has been successfull, so any subsequent locations
listed in the paths variable are not read. This has the effect that the file
from the home directory has the highest priority too like in the previous
example, but without any keys from the system files. Note the ``SHCONF_ONE``
flag.

.. code-block:: c

	const char *home = getenv("HOME");
	const char *paths[] = {home, "/usr/local/etc", "/etc", NULL};
	HX_shconfig_pv(paths, "test.cf", options_table, SHCONF_ONE);
