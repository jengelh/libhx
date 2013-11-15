# -*- Makefile -*-

if BUILD_DOCS
dist_doc_DATA = libHX_Documentation.pdf
endif

libHX_Documentation.pdf: libHX_Documentation.lyx
	srcdir="${srcdir}" ${srcdir}/generate;

EXTRA_DIST = libHX_Documentation.lyx api.txt changelog.txt
