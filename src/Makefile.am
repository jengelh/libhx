# -*- Makefile -*-

AM_CFLAGS        = -DLIBHX_INTERNAL $(regular_CFLAGS) $(GCC_FVISIBILITY_HIDDEN)
DEFAULT_INCLUDES = -I${top_srcdir} -I${top_builddir}

lib_LTLIBRARIES  = libHX.la
libHX_la_SOURCES = arbtree.c deque.c dir.c dl.c format.c hmc.c opt.c other.c \
                   rand.c string.c
libHX_la_LIBADD  = $(regular_LIBS)
libHX_la_LDFLAGS = -version-info 10:0:0

noinst_PROGRAMS    = tc-compile tx-compile
tc_compile_LDADD   = libHX.la
tx_compile_SOURCES = tx-compile.cpp
tx_compile_LDADD   = libHX.la
