# -*- Makefile -*-

AUTOMAKE_OPTIONS = foreign subdir-objects
SUBDIRS          = src

pkgconfigdir     = @pkgconfigdir@
pkgconfig_DATA   = libHX.pc

nobase_include_HEADERS = libHX.h libHX/arbtree.h libHX/clist.h libHX/defs.h \
	libHX/deque.h libHX/list.h libHX/misc.h libHX/option.h libHX/string.h \
	libHX/libxml_helper.h

$(pkgconfig_DATA): config.status

.PHONY: tarball
tarball:
	rm -Rf /tmp/${PACKAGE_NAME}-${PACKAGE_VERSION};
	pushd ${top_srcdir} && git archive --prefix=${PACKAGE_NAME}-${PACKAGE_VERSION}/ HEAD | tar -C /tmp -x && popd;
	pushd /tmp/${PACKAGE_NAME}-${PACKAGE_VERSION} && ./autogen.sh && popd;
	tar -C /tmp -cjf ${PACKAGE_NAME}-${PACKAGE_VERSION}.tar.bz2 --owner=root --group=root ${PACKAGE_NAME}-${PACKAGE_VERSION}/;
	rm -Rf /tmp/${PACKAGE_NAME}-${PACKAGE_VERSION};
