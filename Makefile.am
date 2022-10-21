# -*- Makefile -*-

PACKAGE          = ${PACKAGE_NAME}
ACLOCAL_AMFLAGS  = -I m4
SUBDIRS          = include src

pkgconfig_DATA   = libHX.pc

${pkgconfig_DATA}: ${top_builddir}/config.status

tmpdir := $(shell mktemp -dtu)
packer  = xz
packext = .tar.xz

.PHONY: tarball
tarball:
# do not use mkdir_p here.
	mkdir ${tmpdir}
	cd ${top_srcdir} && git archive --prefix=${PACKAGE_NAME}-${PACKAGE_VERSION}/ HEAD | tar -C ${tmpdir} -x
	cd ${tmpdir}/${PACKAGE_NAME}-${PACKAGE_VERSION} && autoreconf -fi && rm -Rf autom4te.cache
	tar --use=${packer} -C ${tmpdir} -cf ${PACKAGE_NAME}-${PACKAGE_VERSION}${packext} --owner=root --group=root ${PACKAGE_NAME}-${PACKAGE_VERSION}/
	rm -Rf ${tmpdir}
