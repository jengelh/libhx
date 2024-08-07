# -*- Makefile -*-

AM_CPPFLAGS = ${regular_CPPFLAGS} -I${top_srcdir}/include
AM_CFLAGS   = ${regular_CFLAGS}
AM_CXXFLAGS = ${regular_CXXFLAGS}

lib_LTLIBRARIES  = libHX.la
libHX_la_SOURCES = deque.c dl.c format.c io.c map.c \
                   mc.c misc.c opt.c proc.c \
                   rand.c socket.c string.c time.c
libHX_la_LIBADD  = ${libdl_LIBS} -lm ${libpthread_LIBS} ${librt_LIBS} ${libsocket_LIBS}
libHX_la_LDFLAGS = -no-undefined -version-info 40:0:8
if WITH_GNU_LD
libHX_la_LDFLAGS += -Wl,--version-script=${srcdir}/libHX.map
endif
if WITH_SUN_LD
libHX_la_LDFLAGS += -Wl,-M,${srcdir}/libHX.map
endif
EXTRA_libHX_la_DEPENDENCIES = libHX.map

if MINGW32
libHX_la_SOURCES += ux-file.c ux-mmap.c
libHX_la_LIBADD += -lws2_32
endif

EXTRA_DIST = internal.h map_int.h libHX.map uxcompat.h analyze.sh

check_PROGRAMS     = tc-compile tc-cast tc-deque tc-dir tc-format tc-io \
                     tc-list tc-list2 tc-map tc-memmem tc-misc tc-netio \
                     tc-option tc-proc tc-rand tc-realpath \
                     tc-shconfig tc-socket tc-strchr2 tc-string tc-strquote \
                     tc-switchuser tc-time
TESTS              = tc-format tc-option tc-strchr2 tc-string tc-strquote
tc_cast_CFLAGS     = ${AM_CFLAGS} -std=gnu99
tc_cast_LDADD      = libHX.la -lm
tc_compile_LDADD   = libHX.la
tc_dir_LDADD       = libHX.la
tc_format_LDADD    = libHX.la
tc_io_LDADD        = libHX.la
tc_list_LDADD      = libHX.la
tc_list2_LDADD     = libHX.la
tc_list2_CFLAGS    = ${AM_CFLAGS} -O2 -fstrict-aliasing
tc_map_LDADD       = libHX.la -lm
tc_memmem_LDADD    = libHX.la
tc_misc_LDADD      = libHX.la
tc_netio_LDADD     = libHX.la ${libsocket_LIBS}
tc_option_LDADD    = libHX.la
tc_proc_LDADD      = libHX.la
tc_rand_LDADD      = libHX.la
tc_realpath_LDADD  = libHX.la
tc_shconfig_LDADD  = libHX.la
tc_socket_LDADD    = libHX.la
tc_strchr2_LDADD   = libHX.la
tc_string_LDADD    = libHX.la
tc_strquote_LDADD  = libHX.la
tc_switchuser_LDADD = libHX.la
tc_time_LDADD      = libHX.la

if HAVE_CXX
check_PROGRAMS    += tx-compile tx-cast tx-deque tx-dir \
                     tx-intdiff tx-list tx-list2 \
                     tx-misc tx-netio \
                     tx-option tx-proc tx-rand tx-strchr2 tx-string \
                     tx-strquote tx-time
TESTS             += tx-strchr2 tx-strquote
tx_cast_SOURCES    = tx-cast.cpp
tx_cast_LDADD      = libHX.la -lm
tx_compile_SOURCES = tx-compile.cpp
tx_compile_LDADD   = libHX.la
tx_deque_SOURCES   = tx-deque.cpp
tx_dir_SOURCES     = tx-dir.cpp
tx_dir_LDADD       = libHX.la
tx_intdiff_SOURCES = tx-intdiff.cpp
tx_list_SOURCES    = tx-list.cpp
tx_list_LDADD      = libHX.la
tx_list2_SOURCES   = tx-list2.cpp
tx_list2_CXXFLAGS  = ${AM_CXXFLAGS} -O2 -fstrict-aliasing
tx_list2_LDADD     = libHX.la
tx_misc_SOURCES    = tx-misc.cpp
tx_misc_LDADD      = libHX.la
tx_netio_SOURCES   = tx-netio.cpp
tx_netio_LDADD     = libHX.la ${libsocket_LIBS}
tx_option_SOURCES  = tx-option.cpp
tx_option_LDADD    = libHX.la
tx_proc_SOURCES    = tx-proc.cpp
tx_proc_LDADD      = libHX.la
tx_rand_SOURCES    = tx-rand.cpp
tx_rand_LDADD      = libHX.la ${librt_LIBS}
tx_strchr2_SOURCES = tx-strchr2.cpp
tx_strchr2_LDADD   = libHX.la
tx_string_SOURCES  = tx-string.cpp
tx_string_LDADD    = libHX.la
tx_strquote_SOURCES = tx-strquote.cpp
tx_strquote_LDADD   = libHX.la
tx_time_SOURCES    = tx-time.cpp
tx_time_LDADD      = libHX.la ${librt_LIBS}
endif
