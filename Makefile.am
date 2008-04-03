# -*- Makefile -*-

AUTOMAKE_OPTIONS = foreign subdir-objects
SUBDIRS          = src

pkgconfigdir     = @pkgconfigdir@
pkgconfig_DATA   = libHX.pc

nobase_include_HEADERS = libHX.h libHX/arbtree.h libHX/deque.h \
	libHX/option.h libHX/string.h

$(pkgconfig_DATA): config.status
