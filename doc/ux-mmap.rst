======================================
ux-mmap - Unix compatibility functions
======================================

Date authored: 2006-02-25

Description
===========

libHX provides a Linux-style ``mmap()`` function for Win32.

Synopsis
========

.. code-block:: c

	#include <libHX.h>

	void *mmap(void *start, size_t length, int prot, int flags, int fd, off_t offset);

``mmap()``

	See the Linux manual page ``mmap``(2) for details. Many flags described
	in ``mmap``(2) do not work. What does work: ``PROT_NONE``,
	``PROT_READ``, ``PROT_WRITE``, ``PROT_EXEC`` (only WinXP SP2 or
	WinServer2003 SP1 or up), ``MAP_SHARED`` and ``MAP_PRIVATE``.
	``MAP_SHARED`` is the default if no ``MAP_PRIVATE`` is given.
