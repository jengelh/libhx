v4.18 (2023-11-27)
==================

Enhancements:

* opt: new HX_getopt5 API for the parser with untangled in and out variables

Fixes:

* string: HX_strtoull_units handles negative values now (like strtoull)
* string: HX_strtoull_units & HX_strtoull_(n)sec now set errno=ERANGE for
  nonrepresentable results

Behavioral changes:

* string: HX_strtoull_sec rejects unitless numbers now


v4.17 (2023-11-12)
==================

Changes:

* socket: set SOCK_CLOEXEC on all sockets


v4.16 (2023-11-02)
==================

Enhancements:

* Add ``HX_strtoull_nsec`` to parse time period and emit nanoseconds
* doc: specify return value and semantics for ``HXio_fullread``,
  ``HXio_fullwrite``, ``HX_sendfile``


v4.15 (2023-09-24)
==================

Enhancements:

* Add functions to compute Least Positive Residue (``HX_flpr``, ``HX_flprf``)

Fixes:

* Make ``HX_strrtrim`` work on strings longer than ``INT_MAX``


v4.14 (2023-07-14)
==================

Fixes:

* socket: make ``HX_addrport_split`` work on portless bracketed hostspec


v4.13 (2023-06-21)
==================

Fixes:

* io: do not fail ``HX_mkdir`` when a component is a symlink to a directory
* xml_helper: fix infinite recursion in ``xml_getnsprop``


v4.12 (2023-02-27)
==================

Fixes:

* Plug a memory leak in ``HX_inet_listen``


v4.11 (2023-02-26)
==================

Enhancements:

* socket: add ``HX_addrport_split``, ``HX_inet_connect``, ``HX_inet_listen``,
  ``HX_local_listen``


v4.10 (2023-01-29)
==================

Fixes:

* format: plug a memory leak relating to ``func_entry_clone``
* Resolve mingw build failure


v4.9 (2023-01-23)
=================

Enhancements:

* socket: add ``sockaddr_is_local``, ``ipaddr_is_local`` functions

Fixes:

* format: avoid return value truncation from ``HXformat_aprintf``,
  ``HXformat_sprintf``
* format: avoid calling ``HXmc_length`` on a non-hxmc object
* format: add new variations of printf functions returning ``ssize_t``
* Resolve Coverity-SCAN reports


v4.8 (2022-12-03)
=================

Enhancements:

* io: call ``posix_fadvise`` when slurping files

Fixes:

* io: fix garbling of slurped data when read from an unknown-size source


v4.7 (2022-10-21)
=================

Enhancements:

* string: new quoting modes ``HXQUOTE_BASE64URL`` & ``HXQUOTE_BASE64IMAP``

Fixes:

* socket: make ``HX_socket_from_env`` functional on OpenBSD


v4.6 (2022-06-27)
=================

Enhancements:

* ``HX_slurp_fd``/``HX_slurp_file`` now supports reading from files reporting
  their own size as 0 (e.g. ttys, ``/proc`` special files).


v4.5 (2022-04-10)
=================

Fixes:

* Resolve a number of cov-scan reported warnings.


v4.4 (2022-03-15)
=================

Fixes:

* Build fixes for the mingw environment.


v4.3 (2022-03-14)
=================

Enhancements:

* string: New functions ``HX_strtoull_sec``, ``HX_unit_seconds`` for converting
  between second-based time durations and human-readable durations like
  129600 <-> 1d12h.
* io: New function ``HX_sendfile``.
* io: raise buffer size for ``HX_copy_file`` from 1 kiB to 64 kiB


v4.2 (2021-10-17)
=================

Enhancements:

* string: New functions ``HX_strtod_unit``, ``HX_strtoull_unit``,
  ``HX_unit_size``, ``HX_unit_size_cu`` for converting between
  sizes and human-readable sizes like 1457664 <-> "1.45M"/"1.39M".


v4.1 (2021-10-13)
=================

Fixes:

* io: fix a use-after-free in conjunction with ``HX_realpath`` /
  fix missing NULLing of a pointer within ``HX_readlink``


v4.0 (2021-10-03)
=================

Enhancements:

* lib: add ``HX_slurp_fd``, ``HX_slurp_file``
* proc: add ``HXproc_switch_user``
* proc: add ``HXproc_top_fd``
* socket: add ``HX_socket_from_env``
* opt: add ``HXOPT_KEEP_ARGV`` flag

Fixes:

* proc: re-close pipes when ``HXproc_build_pipes`` failed
