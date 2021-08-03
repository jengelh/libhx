
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
