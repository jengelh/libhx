# -*- Makefile -*-

AUTOMAKE_OPTIONS = foreign subdir-objects
SUBDIRS          = src
EXTRA_SUBDIRS    = t

pkgconfigdir     = @pkgconfigdir@
pkgconfig_DATA   = libHX.pc

$(pkgconfig_DATA): config.status
