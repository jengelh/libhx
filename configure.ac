
MAJOR_VERSION=0
MINOR_VERSION=10
AC_INIT(libHX, 0.10)
AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE
AC_PROG_CC
AC_DISABLE_STATIC
AC_LIBTOOL_DLOPEN
AM_PROG_LIBTOOL
AC_CHECK_FUNCS(fork)
AM_CFLAGS="-D_LARGEFILE_SOURCE=1 -D_LARGE_FILES -D_FILE_OFFSET_BITS=64 \
	-D_REENTRANT -Wall -Waggregate-return -Wmissing-declarations \
	-Wmissing-prototypes -Wredundant-decls -Wshadow -Wstrict-prototypes \
	-Winline -pipe"
AC_SUBST(AM_CFLAGS)
AC_OUTPUT(Makefile src/Makefile t/Makefile)
