============
Installation
============

libHX uses GNU autotools as a build environment, which means that
all you have to run as a end-user is the configure with any
options that you need, plus the usual make and make install as
desired.

Pay attention to multi-lib Linux distributions where you most
likely need to specify a different libdir instead of using the
default “lib”. In case of the Debian-style multi-arch/multi-lib
proposal (http://wiki.debian.org/Multiarch)::

.. code-block:: sh

	./configure --libdir='${prefix}/lib/x86_64-linux-gnu'

and the classic-style 32-64 2-lib distributions::

.. code-block:: sh

	./configure --libdir='${prefix}/lib64'

Requirements
------------

* GNU C Compiler 3.3.5 or newer. Other compilers (non-GCC) have
  not been tested in months — use at your own risk.

* approximately 80–160 KB of disk space on Linux for the shared
  library (depends on platform) and header files.

A C++ compiler is only needed if you want to build the C++ test
programs that come with libHX. By default, if there is no C++
compiler present, these will not be built.

* No external libraries are needed for compilation of libHX.
  Helper files, like libxml_helper.h, may reference their include
  files, but they are not used during compilation.


Portability notice
==================

libHX runs on contemporary versions of Linux and Windows. It ought to work on
Solaris and the BSD distributions, but this is not build-tested at this time.

C99 is mandatory. The integer type ``int`` should at best have 32 bits at
least.
