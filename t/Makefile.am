# -*- Makefile -*-

EXTRA_PROGRAMS   = test_btree test_dir test_format test_hpp test_multiline \
                   test_opt test_str test_strncat test_strsep
AM_CFLAGS        = $(regular_CFLAGS) -I$(top_srcdir)/src
AM_CXXFLAGS      = -Wall -pipe -I$(top_srcdir)/src
LDADD            = ../src/libHX.la
test_btree_LDADD = ../src/libHX.la -lm
test_hpp_LDADD   = ../src/libHX.la -ldl
