
AC_INIT(libHX, 1.10)
AC_CONFIG_HEADERS(config.h)
AM_INIT_AUTOMAKE
AC_PROG_CC
AC_PROG_CXX
AC_DISABLE_STATIC
AC_LIBTOOL_DLOPEN
AM_PROG_LIBTOOL

m4_sinclude(m4lib/gcc4_visibility.m4)
AC_HAVE_HEADERS(alloca.h)
AC_CHECK_LIB([dl], [dlopen], [AC_LDLIBS="$AC_LDLIBS -ldl"])
AC_CFLAGS="-D_LARGEFILE_SOURCE=1 -D_LARGE_FILES -D_FILE_OFFSET_BITS=64 \
	-D_REENTRANT -Wall -Waggregate-return -Wmissing-declarations \
	-Wmissing-prototypes -Wredundant-decls -Wshadow -Wstrict-prototypes \
	-Winline -pipe"
AC_SUBST(AC_CFLAGS)
AC_SUBST(AC_LDLIBS)
AC_OUTPUT(Makefile src/Makefile t/Makefile)
