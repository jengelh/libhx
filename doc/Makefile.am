# -*- Makefile -*-

if BUILD_DOCS
dist_doc_DATA = libHX_Documentation.pdf
endif

libHX_Documentation.pdf: libHX_Documentation.lyx
	srcdir="${srcdir}" ${srcdir}/generate;

EXTRA_DIST = libHX_Documentation.lyx api.txt changelog.txt

install-data-local:
	if test -e libHX_Documentation.pdf; then \
		${MKDIR_P} "${DESTDIR}${docdir}" || exit 1; \
		${INSTALL_DATA} libHX_Documentation.pdf "${DESTDIR}${docdir}" || exit $$?; \
	fi
