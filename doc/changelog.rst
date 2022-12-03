v4.8 (2022-12-03)
=================

Enhancements:

* io: call posix_fadvise when slurping files

Fixes:

* io: fix garbling of slurped data when read from an unknown-size source


v4.7 (2022-10-21)
=================

Enhancements:

* string: new quoting modes HXQUOTE_BASE64URL & HXQUOTE_BASE64IMAP

Fixes:

* socket: make HX_socket_from_env functional on OpenBSD


v4.6 (2022-06-27)
=================

Enhancements:

* HX_slurp_fd/HX_slurp_file now supports reading from files reporting their
  own size as 0 (e.g. ttys, /proc special files).


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

* io: fix a use-after-free in conjunction with HX_realpath /
  fix missing NULLing of a pointer within HX_readlink


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


v3.26 (2021-08-03)
==================

Fixes:

* io: cure a potential infinite loop on EOF with HXio_fullread()
* io: HXio_fullread() now returns actual bytes read rather than bytes requested
* time: rectified HX_timeval_sub producing wrong results

Changes:

* nullptr checks were added to HXshconfig_free, HXformat_free, HXdeque_free and
  HXmap_free to make their behavior be in line with free(3).
* Documentation has been switched to reStructured Text.


v3.25 (2020-05-14)
==================

Fixes:

* string: fix out-of-bounds access when calling ``HX_strlcpy(x,y,0)``

Changes:

* string: ``HX_split4`` renamed to ``HX_split_inplace``
* string: ``HX_split5`` renamed to ``HX_split_fixed``
* defs.h: removed partially implementation of ``FIELD_SIZEOF``
* defs.h: removed custom ``offsetof`` definition; you will need to include
  ``<stddef.h>`` or ``<cstddef>`` now.


v3.24 (2018-10-17)
==================

Fixes:

* defs: avoid compiler warning when using ``HX_list_for_each`` in C++
* opt: synchronize ``HXOPT_AUTOHELP`` C behavior to C++ mode


v3.23 (2018-08-28)
==================

Enhancements:

* opt: the option parser now recognizes long option abbreviations
* io: use modern ``readdir`` rather than ``readdir_r``


v3.22 (2014-08-25)
==================

Enhancements:

* string: add the ``HXQUOTE_SQLBQUOTE`` quoting variant
