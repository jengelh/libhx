# -*- Makefile -*-

AM_CFLAGS        = ${regular_CFLAGS} ${GCC_FVISIBILITY_HIDDEN}
DEFAULT_INCLUDES = -I${top_srcdir} -I${top_builddir}

lib_LTLIBRARIES  = libHX.la
libHX_la_SOURCES = arbtree.c deque.c dir.c dl.c format.c hmc.c opt.c other.c \
                   rand.c string.c
libHX_la_LIBADD  = ${regular_LIBS}
libHX_la_LDFLAGS = -version-info 13:0:0

noinst_PROGRAMS    = tc-compile tx-compile
noinst_PROGRAMS   += tc-arbtree tx-arbtree
noinst_PROGRAMS   += tc-deque tx-deque
noinst_PROGRAMS   += tc-list tx-list
noinst_PROGRAMS   += tc-misc tx-misc
noinst_PROGRAMS   += tc-option tx-option
noinst_PROGRAMS   += tc-other tx-other
noinst_PROGRAMS   += tc-dir tx-dir
noinst_PROGRAMS   += tc-string tx-string
tc_arbtree_LDADD   = libHX.la -lm
tc_compile_LDADD   = libHX.la
tc_dir_LDADD       = libHX.la
tc_list_LDADD      = libHX.la
tc_option_LDADD    = libHX.la
tc_other_LDADD     = libHX.la
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
tx_option_SOURCES  = tx-option.cpp
tx_other_SOURCES   = tx-other.cpp
tx_other_LDADD     = libHX.la
tx_string_SOURCES  = tx-string.cpp
tx_string_LDADD    = libHX.la
