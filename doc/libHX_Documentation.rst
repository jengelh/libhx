===================
libHX Documentation
===================

libHX collects many useful day-to-day functions, intended to reduce the amount
of otherwise repeatedly open-coded instructions.

Overview
========

* Maps (key-value pairs)

  Originally created to provide a data structure like Perl's associative
  arrays. Different map types and characteristics are available, such as
  hash-based or the traditional rbtree.

* Linked lists (Deques)

  Double-ended queues, implemented as a doubly-linked list with sentinels, are
  suitable for both providing stack and queue functionality.

* Inline doubly-linked list, uncounted and counted

  Light-weight linked lists as used in the Linux kernel.

* Common string operations

  ``basename``, ``chomp``, ``dirname``, ``getl``/``getline``, ``split``,
  ``strlcat``/``strlcpy``, ``strlower``/``strupper``, ``str*trim``, ``strsep``,
  etc.

* Memory container, auto-sizing string operations

  Scripting-like invocation for string handling — automatically doing
  (re)allocations as needed.

* String formatter (Format templates)

  HXfmt is a template system for by-name variable expansion. It can be used to
  substitute placeholders in format strings supplied by the user by appropriate
  expanded values defined by the program.

* Directory functions

  Directory creation, traversal, removal, and file copying.

* Option parsing

  Table-/callback-based option parser that works similar to Perl's
  Getopt::Long — no open-coding but a single “atomic” invocation.

* Shell-style config parser

  Configuration file reader for Shell-style “configuration” files
  with key-value pairs, as usually foudn in ``/etc/sysconfig``.

* Random number gathering

  Convenient wrapper that uses kernel-provided RNG devices when
  available.

* External process invocation

  Setting up pipes for the standard file descriptors for
  sending/capturing data to/from a program.

* a bit more beyond that ... Miscellaneous


General reading
===============

* `Installation <install.rst>`_ (if you do not have a binray package from a
  distro yet)
* `History <history.rst>`_ remarks


First things first
==================

Many functions are prefixed with ``HX_`` or ``HXsubsys_``, as are structures
(sometimes without underscores, be sure to check the syntax and names), to
avoid name clashes with possibly existing files. Functions that are not tied to
a specific data structure such as most of the string functions use the
subsystem-less prefix, ``HX_``. Functions from a clearly-defined subsystem,
such as map or deque, augment the base prefix by a suffix, forming e.g.
``HXmap_``.

* `Library initialization <init.rst>`_


Data structures
===============

* `Bitmaps <bitmaps.rst>`_
* `Maps <maps.rst>`_
* `Linked lists <linked_list.rst>`_
* `Inline lists <inline_list.rst>`_
* `Counted inline lists <inline_clist.rst>`_


Strings and memory
==================

* `String operations <string_ops.rst>`_
* `Memory containers <memory_container.rst>`_
* `String formatter <string_formatter.rst>`_
* Everything with `Files and directories <files_and_dirs.rst>`_


Options and configuration files
===============================

* `Option parsing <option_parsing.rst>`_
* `Shell-style config file parser <shconfig.rst>`_


Systems-related components
==========================

* `Time functions <time_functions.rst>`_
* `Random numbers <random_number.rst>`_
* `Process functions <process_functions.rst>`_


Miscellaneous
=============

* `Type-checking casts <typechecking_casts.rst>`_
* `Helper headers <helper_headers.rst>`_ for other software
* `Macros <macros.rst>`_
* `Misc functions <misc_functions.rst>`_ that did not fit in any other category


Resources
=========

As of this writing, the repository is located at

* git://git.inai.de/libhx — clone URL

* https://inai.de/projects/libhx/ — home page (and link to tarballs)
