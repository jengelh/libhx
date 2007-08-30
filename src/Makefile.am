# -*- Makefile -*-

AM_CFLAGS        = -DLIBHX_INTERNAL $(regular_CFLAGS) $(GCC_FVISIBILITY_HIDDEN)

lib_LTLIBRARIES  = libHX.la
libHX_la_SOURCES = arbtree.c deque.c dir.c dl.c format.c hmc.c opt.c other.c \
                   rand.c string.c
libHX_la_LIBADD  = $(regular_LIBS)
libHX_la_LDFLAGS = -module -version-info 10:0:0
include_HEADERS  = libHX.h
