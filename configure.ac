AC_INIT([libHX], [3.25])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_INSTALL
AM_INIT_AUTOMAKE([-Wall -Wno-override foreign subdir-objects
	tar-pax no-dist-gzip dist-xz])
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
AC_DISABLE_STATIC
AC_LIBTOOL_DLOPEN
AM_PROG_LIBTOOL

#
# Check whether there really is a C++ compiler.
# It is not mandatory to compile libHX, but we want to know.
#
AC_LANG_PUSH([C++])
AC_COMPILE_IFELSE(
	[AC_LANG_PROGRAM([], [return 0;])],
	[ac_cv_cxx=yes], [ac_cv_cxx=no])
AC_LANG_POP([C++])
AM_CONDITIONAL([HAVE_CXX], [test "$ac_cv_cxx" = yes])

AC_MSG_CHECKING([for mingw32])
AC_LANG_PUSH([C])
AC_COMPILE_IFELSE(
	[AC_LANG_PROGRAM([], [return __MINGW32__;])],
	[ac_cv_mingw32=yes; AC_MSG_RESULT([yes])],
	[ac_cv_mingw32=no; AC_MSG_RESULT([no])]
)
AC_LANG_POP([C])
AM_CONDITIONAL([MINGW32], [test "$ac_cv_mingw32" = yes])

AC_ARG_WITH([pkgconfigdir], AS_HELP_STRING([--with-pkgconfigdir=PATH],
	[Path to the pkgconfig directory [[LIBDIR/pkgconfig]]]),
	[pkgconfigdir="$withval"], [pkgconfigdir='${libdir}/pkgconfig'])
AC_SUBST([pkgconfigdir])

CHECK_GCC_FVISIBILITY
AM_CONDITIONAL([WITH_GNU_LD], [test "$with_gnu_ld" = yes])

LIBS_system="$LIBS"
AC_CHECK_HEADERS([dlfcn.h sys/resource.h])
AM_CONDITIONAL([HAVE_DLFCN_H], [test "$ac_cv_header_dlfcn_h" = "yes"])
AC_SEARCH_LIBS([dlopen], [dl], [libdl_LIBS="$LIBS"; LIBS="$LIBS_system"])
AC_SUBST([libdl_LIBS])
# glibc < 2.17 has it in librt, mingw-w64 has it in libpthread
AC_SEARCH_LIBS([clock_gettime], [rt pthread], [librt_LIBS="$LIBS"; LIBS="$LIBS_system"])
AC_SUBST([librt_LIBS])
AC_SEARCH_LIBS([pthread_mutex_lock], [pthread], [libpthread_LIBS="$LIBS"; LIBS="$LIBS_system"])
AC_SUBST([libpthread_LIBS])

libsocket_ok=0
AC_SEARCH_LIBS([getaddrinfo], [socket],
	[libsocket_LIBS="$LIBS"; LIBS="$LIBS_system"; libsocket_ok=1], [])
if test $libsocket_ok = 0; then
	dnl socket: Need to distinguish between "none required" and
	dnl "no"(ne found), and only in the latter case try ws2_32.
	dnl
	dnl AC_SEARCH_LIBS is too dumb, we really need the ws2tcpip
	dnl include to get a functional prototype (which is stdcall, sadly)
	dnl for successful linking.
	AC_MSG_CHECKING([non-standard libraries containing getaddrinfo])
	LIBS="$LIBS_system -lws2_32";
	AC_LINK_IFELSE([AC_LANG_SOURCE([
		#include <winsock2.h>
		#include <ws2tcpip.h>
		int main(void) { return getaddrinfo(NULL, NULL, NULL, NULL); }
	])], [
		AC_MSG_RESULT([-lws2_32])
		libsocket_LIBS="$libsocket_LIBS $LIBS";
	], [
		AC_MSG_RESULT([no])
	])
	LIBS="$LIBS_system";
fi;
AC_SUBST([libsocket_LIBS])

AC_CHECK_SIZEOF([void *])
AC_CHECK_SIZEOF([char *])
AC_CHECK_SIZEOF([struct x *])
AC_CHECK_SIZEOF([struct x **])
if test "$ac_cv_sizeof_void_p" != "$ac_cv_sizeof_char_p" ||
   test "$ac_cv_sizeof_struct_x_p" != "$ac_cv_sizeof_struct_x_pp"; then
	# format.c: reinterpret_cast(hxmc_t **, HXdeque_to_vec(...))
	# map.c: reinterpret_cast(struct HXrbtree_node *, &btree->root);
	AC_MSG_ERROR([The code currently relies on certain type sizes to take shortcuts.])
fi;

AC_CHECK_MEMBERS(
	[struct timespec.tv_nsec,
	struct timeval.tv_usec,
	struct stat.st_mtimensec, struct stat.st_mtim,
	struct stat.st_mtimespec, struct stat.st_mtime,
	struct stat.st_otimensec, struct stat.st_otim,
	struct stat.st_otimespec, struct stat.st_otime],
	[], [], [
	#include <sys/stat.h>
	#include <sys/time.h>
	#include <time.h>
	])
AC_CHECK_FUNCS([fork execv execvp pipe], [b_proc="1"])
AC_CHECK_FUNCS([getegid geteuid getpid getppid])
AM_CONDITIONAL([B_PROC], [test "$b_proc" = 1])

AC_CHECK_PROGS([LYX], [lyx])
AM_CONDITIONAL([BUILD_DOCS], [test -n "$LYX"])

regular_CPPFLAGS="-D_FILE_OFFSET_BITS=64 -D_REENTRANT"
regular_CFLAGS="-Wall -Waggregate-return -Wmissing-declarations \
	-Wmissing-prototypes -Wredundant-decls -Wshadow -Wstrict-prototypes \
	-Wformat=2 -pipe $visibility_CFLAGS"
regular_CXXFLAGS="-Wall -Wno-pointer-arith -Wredundant-decls -pipe \
	$visibility_CFLAGS";
AC_SUBST([regular_CPPFLAGS])
AC_SUBST([regular_CFLAGS])
AC_SUBST([regular_CXXFLAGS])
AC_CONFIG_FILES([Makefile assorted/Makefile doc/Makefile src/Makefile
	include/Makefile libHX.pc])
AC_OUTPUT
