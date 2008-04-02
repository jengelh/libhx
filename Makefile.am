# -*- Makefile -*-

AUTOMAKE_OPTIONS = foreign subdir-objects
SUBDIRS          = src
EXTRA_SUBDIRS    = t

pkgconfigdir     = @pkgconfigdir@
pkgconfig_DATA   = libHX.pc

nobase_include_HEADERS = libHX.h libHX/arbtree.h libHX/deque.h

$(pkgconfig_DATA): config.status
