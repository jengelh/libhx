# -*- Makefile -*-

noinst_PROGRAMS    = test_btree test_dir test_format test_hpp test_multiline \
                     test_opt test_str test_strncat test_strsep
AM_CFLAGS          = $(AC_CFLAGS) -I$(top_srcdir)/src
AM_CXXFLAGS        = -Wall -pipe -I$(top_srcdir)/src
AM_LDFLAGS         = ../src/libHX.la
test_btree_LDFLAGS = -lm
test_hpp_SOURCES   = test_hpp.cpp
test_hpp_LDFLAGS   = -ldl
