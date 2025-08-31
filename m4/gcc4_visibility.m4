
# GCC 4.x -fvisibility=hidden

AC_DEFUN([CHECK_GCC_FVISIBILITY], [
	AC_LANG_PUSH([C])
	saved_CFLAGS="$CFLAGS"
	CFLAGS="$saved_CFLAGS -fvisibility=hidden"
	AC_CACHE_CHECK([whether compiler accepts -fvisibility=hidden],
	  [ac_cv_fvisibility_hidden], AC_COMPILE_IFELSE(
		[AC_LANG_SOURCE()],
		[ac_cv_fvisibility_hidden=yes],
		[ac_cv_fvisibility_hidden=no]
	))
	if test "$ac_cv_fvisibility_hidden" = "yes"; then
		AC_DEFINE([HAVE_VISIBILITY_HIDDEN], [1],
			[True if compiler supports -fvisibility=hidden])
		visibility_CFLAGS="-fvisibility=hidden"
	fi
	CFLAGS="$saved_CFLAGS"
	AC_LANG_POP([C])
])

AC_DEFUN([CHECK_LD_SYMVERS], [
	AC_MSG_CHECKING([linker support for symbol maps])
	echo '{global:*;};' >conftest.map
	saved_LDFLAGS="$LDFLAGS"
	LDFLAGS="$LDFLAGS -Wl,--version-script=conftest.map"
	AC_LINK_IFELSE([AC_LANG_SOURCE([ int main() { return 0; } ])],
		[with_ldsym=yes], [with_ldsym=no])
	LDFLAGS="$saved_LDFLAGS"
	rm conftest.map
	AM_CONDITIONAL([WITH_LDSYM], [test "$with_ldsym" = yes])
	AS_IF([test -n "$LD" && $LD -z help >/dev/null 2>/dev/null], [with_sun_ld=yes], [with_sun_ld=no])
	AM_CONDITIONAL([WITH_SUN_LD], [test "$with_sun_ld" = yes])
	AS_IF([test "$with_ldsym" = yes], [
		AC_MSG_RESULT([-Wl,--version-script])
	], [test "$with_sun_ld" = yes], [
		AC_MSG_RESULT([-M])
	], [
		AC_MSG_RESULT([no])
	])
])
