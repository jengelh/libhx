
AC_INIT([libHX], [1.27])
AC_CONFIG_HEADERS([libHX/config.h])
AC_PROG_INSTALL
AM_INIT_AUTOMAKE
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AC_DISABLE_STATIC
AC_LIBTOOL_DLOPEN
AM_PROG_LIBTOOL

AC_MSG_CHECKING([for mingw32])
AC_TRY_COMPILE(, [return __MINGW32__;],
	[ac_cv_mingw32=yes; AC_MSG_RESULT([yes])],
	[ac_cv_mingw32=no; AC_MSG_RESULT([no])]
)
AM_CONDITIONAL([MINGW32], [test "$ac_cv_mingw32" = yes])

AC_ARG_WITH([pkgconfigdir], AS_HELP_STRING([--with-pkgconfigdir=PATH],
	[Path to the pkgconfig directory [[LIBDIR/pkgconfig]]]),
	[pkgconfigdir="$withval"], [pkgconfigdir='${libdir}/pkgconfig'])
AC_SUBST(pkgconfigdir)

m4_sinclude(m4lib/gcc4_visibility.m4)
AC_CHECK_LIB([dl], [dlopen], [regular_LIBS="$regular_LIBS -ldl"])

AC_CHECK_MEMBERS(
	[struct stat.st_mtimensec, struct stat.st_mtim,
	struct stat.st_mtimespec, struct stat.st_mtime],
	[], [], [#include <sys/stat.h>])

regular_CFLAGS="-D_LARGEFILE_SOURCE=1 -D_LARGE_FILES -D_FILE_OFFSET_BITS=64 \
	-D_REENTRANT -Wall -Waggregate-return -Wmissing-declarations \
	-Wmissing-prototypes -Wredundant-decls -Wshadow -Wstrict-prototypes \
	-Wformat=2 -pipe"
AC_SUBST(regular_CFLAGS)
AC_SUBST(regular_LIBS)
AC_OUTPUT([Makefile src/Makefile libHX.pc])
