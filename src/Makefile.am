# -*- Makefile -*-

AM_CFLAGS        = ${regular_CFLAGS} ${GCC_FVISIBILITY_HIDDEN}
DEFAULT_INCLUDES = -I${top_srcdir} -I${top_builddir}

lib_LTLIBRARIES  = libHX.la
libHX_la_SOURCES = arbtree.c deque.c dir.c dl.c format.c mc.c misc.c opt.c \
                   rand.c string.c ${mingw32_extra}
libHX_la_LIBADD  = ${regular_LIBS}
libHX_la_LDFLAGS = -version-info 15:0:1

if MINGW32
mingw32_extra    = ux-file.c ux-mmap.c
endif

noinst_PROGRAMS    = tc-compile tx-compile
noinst_PROGRAMS   += tc-arbtree tx-arbtree
noinst_PROGRAMS   += tc-cast
noinst_PROGRAMS   += tc-deque tx-deque
noinst_PROGRAMS   += tc-list tx-list
noinst_PROGRAMS   += tc-misc tx-misc
noinst_PROGRAMS   += tc-option tx-option
noinst_PROGRAMS   += tc-dir tx-dir
noinst_PROGRAMS   += tc-string tx-string
tc_arbtree_LDADD   = libHX.la -lm
tc_compile_LDADD   = libHX.la
tc_dir_LDADD       = libHX.la
tc_list_LDADD      = libHX.la
tc_misc_LDADD      = libHX.la
tc_option_LDADD    = libHX.la
tc_string_LDADD    = libHX.la
tx_arbtree_SOURCES = tx-arbtree.cpp
tx_compile_SOURCES = tx-compile.cpp
tx_compile_LDADD   = libHX.la
tx_deque_SOURCES   = tx-deque.cpp
tx_dir_SOURCES     = tx-dir.cpp
tx_dir_LDADD       = libHX.la
tx_list_SOURCES    = tx-list.cpp
tx_list_LDADD      = libHX.la
tx_misc_SOURCES    = tx-misc.cpp
tx_misc_LDADD      = libHX.la
tx_option_SOURCES  = tx-option.cpp
tx_string_SOURCES  = tx-string.cpp
tx_string_LDADD    = libHX.la
