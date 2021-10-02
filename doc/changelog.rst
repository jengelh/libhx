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
