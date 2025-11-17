# -*- Makefile -*-

PACKAGE          = ${PACKAGE_NAME}
ACLOCAL_AMFLAGS  = -I m4
SUBDIRS          = include src

pkgconfig_DATA   = libHX.pc
EXTRA_DIST = doc/api.rst doc/assorted.txt doc/bitmaps.rst doc/changelog.rst doc/const_cast.c doc/dirstamp.rst doc/files_and_dirs.rst doc/generator.cpp doc/helper_headers.rst doc/history.rst doc/init.rst doc/inline_clist.rst doc/inline_list.rst doc/install.rst doc/libHX_Documentation.rst doc/linked_list.rst doc/macros.rst doc/maps.rst doc/memory_container.rst doc/misc_functions.rst doc/option_parsing.rst doc/process_management.rst doc/random_numbers.rst doc/scope.rst doc/shconfig.rst doc/socket_functions.rst doc/string_formatter.rst doc/string_ops.rst doc/strlcpy-timing.rst doc/tie.rst doc/time_functions.rst doc/typecheck_casts.rst doc/ux-file.rst doc/ux-mmap.rst

${pkgconfig_DATA}: ${top_builddir}/config.status
