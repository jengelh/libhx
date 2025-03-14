Function reference
==================

* R column: Recommend version number to use in ``PKG_CONFIG_CHECK`` in
  projects using libHX. Includes important bugfixes.
* M column: Lowest possible version with the same ABI (minus cv qualification).
* F column: First version that the function name was in use.

======  ======  ======  ========================================
RMV     MinVer  FirstA  Name
======  ======  ======  ========================================
4.25    inline  4.25    HX_isascii
4.25    inline  4.25    HX::make_scope_exit
4.25    inline  4.25    cpu_to_le{16,32,64}p cpu_to_be{16,32,64}p
4.25    inline  4.25    le{16,32,64}p_to_cpu be{16,32,64}p_to_cpu
4.25    4.25    4.25    HXSIZEOF_UNITSEC64
4.24    4.24    4.24    HX_getcwd
4.19    4.18    4.18    HX_getopt5
4.16    4.16    4.16    HX_strtoull_nsec
4.15    4.15    4.15    HX_flpr
4.15    4.15    4.15    HX_flprf
4.11    4.11    4.11    HX_addrport_split
4.11    4.11    4.11    HX_inet_connect
4.11    4.11    4.11    HX_inet_listen
4.11    4.11    4.11    HX_local_listen
4.9     4.9     4.9     HX_sockaddr_is_local
4.9     4.9     4.9     HX_ipaddr_is_local
4.7     4.7     4.7     HXQUOTE_BASE64IMAP
4.7     4.7     4.7     HXQUOTE_BASE64URL
4.3     4.3     4.3     HX_unit_seconds
4.3     4.3     4.3     HX_strtoull_sec
4.2     4.2     4.2     HX_unit_size
4.2     4.2     4.2     HX_unit_size_cu
4.2     4.2     4.2     HX_strtod_unit
4.2     4.2     4.2     HX_strtoull_unit
3.27    3.27    3.27    HXOPT_KEEP_ARGV
3.27    3.27    3.27    HXproc_top_fd
3.27    3.27    3.27    HXproc_switch_user
3.27    3.27    3.27    HXPROC_SU_SUCCESS
3.27    3.27    3.27    HXPROC_SU_NOOP
3.27    3.27    3.27    HXPROC_USER_NOT_FOUND
3.27    3.27    3.27    HXPROC_GROUP_NOT_FOUND
3.27    3.27    3.27    HXPROC_SETUID_FAILED
3.27    3.27    3.27    HXPROC_SETGID_FAILED
3.27    3.27    3.27    HXPROC_INITGROUPS_FAILED
3.27    3.27    3.27    HX_slurp_fd
3.27    3.27    3.27    HX_slurp_file
3.25    3.25    3.25    HX_split_fixed
3.25    3.25    3.25    HX_split_inplace
3.22    3.22    3.22    HXQUOTE_SQLBQUOTE
3.21    3.21    3.21    xml_getnsprop
3.19    3.19    3.19    HXQUOTE_SQLSQUOTE
3.18    3.18    3.18    HX_stpltrim
3.17    3.17    3.17    HX_LONGLONG_FMT
3.17    3.17    3.17    HX_SIZET_FMT
3.16    3.16    3.16    container_of
3.16    3.16    3.16    wxCDF
3.16    3.16    3.16    wxDSPAN
3.15    3.15    3.15    HXQUOTE_URIENC
3.15    3.15    3.15    HX_strchr2
3.13    3.13    3.13    DEMOTE_TO_PTR
3.13    3.13    3.13    HXTYPE_SIZE_T
3.13    3.13    3.13    HX_TIMESPEC_EXP
3.13    3.13    3.13    HX_TIMESPEC_FMT
3.13    3.13    3.13    HX_TIMEVAL_EXP
3.13    3.13    3.13    HX_TIMEVAL_FMT
3.13    3.13    3.13    HX_timespec_add
3.13    3.13    3.13    HX_timespec_isneg
3.13    3.13    3.13    HX_timespec_mul
3.13    3.13    3.13    HX_timespec_mulf
3.13    3.13    3.13    HX_timespec_neg
3.13    3.13    3.13    HX_timespec_sub
3.13    3.13    3.13    HX_timeval_sub
3.12    3.12    1.10.0  HX_mkdir
3.12    3.12    3.12    HX_strndup
3.12    3.12    3.12    HX_strnlen
3.12    3.0     3.0     HXMAP_CDATA
3.12    3.0     3.0     HXMAP_CKEY
3.12    3.0     3.0     HXMAP_SCDATA
3.12    3.0     3.0     HXMAP_SCKEY
3.12    3.0     3.0     HXMAP_SDATA
3.12    3.0     3.0     HXMAP_SINGULAR
3.12    3.0     3.0     HXMAP_SKEY
3.12    3.12    3.12    HXOPT_ERR_SUCCESS
3.12    3.12    3.12    HXOPT_ERR_SYS
3.12    3.12    3.12    HXOPTCB_BY_LONG
3.12    3.12    3.12    HXOPTCB_BY_SHORT
3.12    3.0     1.10.0  HXformat_aprintf
3.12    3.0     1.10.0  HXformat_fprintf
3.12    3.0     1.10.0  HXformat_sprintf
3.11    3.11    3.11    HXQUOTE_BASE64
3.10    3.10    3.10    BUILD_BUG_ON_EXPR
3.10    3.10    3.10    HX_readlink
3.10    3.10    3.10    HX_realpath
3.9.1   3.9     3.9     HXio_fullread
3.9.1   3.9     3.9     HXio_fullwrite
3.9     3.9     3.9     HXMAP_NONE
3.7     3.7     3.7     HXlist_for_each_rev
3.7     3.7     3.7     HXlist_for_each_rev_safe
3.7     3.7     1.22    xml_newnode
3.7     1.15    1.15    HXclist_pop
3.7     1.15    1.15    HXclist_shift
3.7     1.10.0  1.10.0  HX_ffs
3.7     1.10.0  1.10.0  HX_zveclen
3.7     1.10.0  1.10.0  HXdir_close
3.7     1.10.0  1.10.0  HXdir_open
3.7     1.10.0  1.10.0  HXdir_read
3.6     3.6     3.1     HXbitmap_clear
3.6     3.6     3.1     HXbitmap_set
3.6     3.6     3.1     HXbitmap_test
3.6     1.10.0  1.10.0  HX_split
3.5     3.5     3.5     HXMAP_NOFLAGS
3.5     3.5     3.5     HXQUOTE_LDAPFLT
3.5     3.5     3.5     HXQUOTE_LDAPRDN
3.5     3.5     3.5     HXSIZEOF_Z16
3.5     2.2     2.2     HXproc_run_async
3.5     2.2     2.2     HXproc_run_sync
3.4     3.4     3.4     HX_exit
3.4     3.4     3.4     HX_init
3.4     3.4     3.4     HX_memmem
3.4     3.4     3.4     HXlist_empty
3.3     3.3     3.3     HX_drand
3.3     3.3     3.3     HX_shconfig_map
3.3     3.3     3.3     HXdeque_genocide2
3.3     3.3     3.3     HXmc_zvecfree
3.3     1.10.0  1.10.0  HX_shconfig
3.3     1.10.0  1.10.0  HX_shconfig_pv
3.2     3.2     3.2     HXQUOTE_DQUOTE
3.2     3.2     3.2     HXQUOTE_HTML
3.2     3.2     3.2     HXQUOTE_SQUOTE
3.2     3.2     3.2     HXTYPE_MCSTR
3.2     3.2     3.2     HX_strquote
3.1     3.1     3.1     HXbitmap_size
3.1     1.25    1.25    HXmc_strcpy
3.0.1   3.0     3.0     HXmap_add
3.0.1   3.0     3.0     HXmap_del
3.0.1   3.0     3.0     HXmap_del<>
3.0.1   3.0     3.0     HXmap_find
3.0.1   3.0     3.0     HXmap_get
3.0.1   3.0     3.0     HXmap_get<>
3.0.1   3.0     3.0     HXmap_qfe
3.0.1   3.0     3.0     HXmap_traverse
3.0.1   3.0     3.0     HXmap_travinit
3.0     3.0     3.0     HXMAPT_DEFAULT
3.0     3.0     3.0     HXMAPT_HASH
3.0     3.0     3.0     HXMAPT_ORDERED
3.0     3.0     3.0     HXMAPT_RBTREE
3.0     3.0     3.0     HXMAP_DTRAV
3.0     3.0     3.0     HXMAP_NOREPLACE
3.0     3.0     3.0     HXhash_djb2
3.0     3.0     3.0     HXhash_jlookup3
3.0     3.0     3.0     HXhash_jlookup3s
3.0     3.0     3.0     HXmap_free
3.0     3.0     3.0     HXmap_init
3.0     3.0     3.0     HXmap_init5
3.0     3.0     3.0     HXmap_keysvalues
3.0     3.0     3.0     HXmap_travfree
3.0     3.0     3.0     HXsizeof_member
3.0     3.0     3.0     HXtypeof_member
3.0     3.0     1.10.0  HXformat_add
3.0     3.0     1.10.0  HXformat_free
3.0     3.0     1.10.0  HXformat_init
2.9     2.9     2.9     HX_basename_exact
2.9     2.2     2.2     HX_split4
2.9     1.10.0  1.10.0  HX_basename
2.8     2.8     2.8     HXPROC_NULL_STDERR
2.8     2.8     2.8     HXPROC_NULL_STDIN
2.8     2.8     2.8     HXPROC_NULL_STDOUT
2.6     2.6     2.6     HX_fls
2.6     2.6     2.6     wxACV
2.6     2.6     2.6     wxDPOS
2.6     2.6     2.6     wxDSIZE
2.6     2.6     2.6     wxfu8
2.6     2.6     2.6     wxfv8
2.6     2.6     2.6     wxtu8
2.6     2.6     2.6     xml_strcasecmp
2.3     1.25    1.25    HXmc_length
2.2     2.2     2.2     HXPROC_A0
2.2     2.2     2.2     HXPROC_EXECV
2.2     2.2     2.2     HXPROC_STDERR
2.2     2.2     2.2     HXPROC_STDIN
2.2     2.2     2.2     HXPROC_STDOUT
2.2     2.2     2.2     HXPROC_VERBOSE
2.2     2.2     2.2     HXSIZEOF_Z32
2.2     2.2     2.2     HXSIZEOF_Z64
2.2     2.2     2.2     HX_STRINGIFY
2.2     2.2     2.2     HXproc_wait
2.2     2.0     2.0     const_cast1
2.2     2.0     2.0     const_cast2
2.2     2.0     2.0     const_cast3
2.1     2.0     2.0     static_cast
2.0     2.0     2.0     HX_isalnum
2.0     2.0     2.0     HX_isalpha
2.0     2.0     2.0     HX_isdigit
2.0     2.0     2.0     HX_islower
2.0     2.0     2.0     HX_isprint
2.0     2.0     2.0     HX_isspace
2.0     2.0     2.0     HX_isupper
2.0     2.0     2.0     HX_isxdigit
2.0     2.0     2.0     HX_tolower
2.0     2.0     2.0     HX_toupper
2.0     2.0     2.0     HXmc_setlen
2.0     2.0     2.0     const_cast
2.0     2.0     2.0     containerof
2.0     2.0     2.0     reinterpret_cast
2.0     2.0     2.0     signed_cast<>
2.0     1.23    1.23    signed_cast
2.0     1.10.0  1.10.0  HX_strmid
1.28    1.28    1.28    HXTYPE_INT16
1.28    1.28    1.28    HXTYPE_INT32
1.28    1.28    1.28    HXTYPE_INT64
1.28    1.28    1.28    HXTYPE_INT8
1.28    1.28    1.28    HXTYPE_UINT16
1.28    1.28    1.28    HXTYPE_UINT32
1.28    1.28    1.28    HXTYPE_UINT64
1.28    1.28    1.28    HXTYPE_UINT8
1.26    1.26    1.26    HX_hexdump
1.26    1.26    1.26    HX_time_compare
1.25    1.25    1.25    HX_getl
1.25    1.25    1.25    HXmc_free
1.25    1.25    1.25    HXmc_memcat
1.25    1.25    1.25    HXmc_memcpy
1.25    1.25    1.25    HXmc_memdel
1.25    1.25    1.25    HXmc_meminit
1.25    1.25    1.25    HXmc_memins
1.25    1.25    1.25    HXmc_mempcat
1.25    1.25    1.25    HXmc_strcat
1.25    1.25    1.25    HXmc_strinit
1.25    1.25    1.25    HXmc_strins
1.25    1.25    1.25    HXmc_strpcat
1.25    1.25    1.25    HXmc_trunc
1.23    1.23    1.23    ARRAY_SIZE
1.23    1.23    1.23    BUILD_BUG_ON
1.23    1.23    1.23    O_BINARY
1.23    1.23    1.23    S_IRUGO
1.23    1.23    1.23    S_IRWXUGO
1.23    1.23    1.23    S_IWUGO
1.23    1.23    1.23    S_IXUGO
1.22    1.22    1.22    xml_getprop
1.22    1.22    1.22    xml_newprop
1.22    1.22    1.22    xml_strcmp
1.18    1.18    1.18    HXlist_for_each_entry_rev
1.17    1.17    1.17    HXclist_del
1.17    1.17    1.17    HXlist_entry
1.17    1.17    1.17    HXlist_for_each_entry_safe
1.17    1.17    1.17    HXlist_for_each_safe
1.17    1.17    1.15    HXclist_init
1.17    1.17    1.15    HXlist_init
1.15    1.15    1.15    HXCLIST_HEAD
1.15    1.15    1.15    HXCLIST_HEAD_INIT
1.15    1.15    1.15    HXLIST_HEAD
1.15    1.15    1.15    HXLIST_HEAD_INIT
1.15    1.15    1.15    HXclist_push
1.15    1.15    1.15    HXclist_unshift
1.15    1.15    1.15    HXlist_add
1.15    1.15    1.15    HXlist_add_tail
1.15    1.15    1.15    HXlist_del
1.15    1.15    1.15    HXlist_for_each
1.15    1.15    1.15    HXlist_for_each_entry
1.10.0  1.10.0  1.10.0  HXFORMAT_IMMED
1.10.0  1.10.0  1.10.0  HXF_GID
1.10.0  1.10.0  1.10.0  HXF_KEEP
1.10.0  1.10.0  1.10.0  HXF_UID
1.10.0  1.10.0  1.10.0  HXOPT_AND
1.10.0  1.10.0  1.10.0  HXOPT_AUTOHELP
1.10.0  1.10.0  1.10.0  HXOPT_DEC
1.10.0  1.10.0  1.10.0  HXOPT_DESTROY_OLD
1.10.0  1.10.0  1.10.0  HXOPT_ERR_MIS
1.10.0  1.10.0  1.10.0  HXOPT_ERR_UNKN
1.10.0  1.10.0  1.10.0  HXOPT_ERR_VOID
1.10.0  1.10.0  1.10.0  HXOPT_HELPONERR
1.10.0  1.10.0  1.10.0  HXOPT_INC
1.10.0  1.10.0  1.10.0  HXOPT_NOT
1.10.0  1.10.0  1.10.0  HXOPT_OPTIONAL
1.10.0  1.10.0  1.10.0  HXOPT_OR
1.10.0  1.10.0  1.10.0  HXOPT_PTHRU
1.10.0  1.10.0  1.10.0  HXOPT_QUIET
1.10.0  1.10.0  1.10.0  HXOPT_TABLEEND
1.10.0  1.10.0  1.10.0  HXOPT_USAGEONERR
1.10.0  1.10.0  1.10.0  HXOPT_XOR
1.10.0  1.10.0  1.10.0  HXTYPE_BOOL
1.10.0  1.10.0  1.10.0  HXTYPE_CHAR
1.10.0  1.10.0  1.10.0  HXTYPE_DOUBLE
1.10.0  1.10.0  1.10.0  HXTYPE_FLOAT
1.10.0  1.10.0  1.10.0  HXTYPE_INT
1.10.0  1.10.0  1.10.0  HXTYPE_LLONG
1.10.0  1.10.0  1.10.0  HXTYPE_LONG
1.10.0  1.10.0  1.10.0  HXTYPE_NONE
1.10.0  1.10.0  1.10.0  HXTYPE_SHORT
1.10.0  1.10.0  1.10.0  HXTYPE_STRDQ
1.10.0  1.10.0  1.10.0  HXTYPE_STRING
1.10.0  1.10.0  1.10.0  HXTYPE_STRP
1.10.0  1.10.0  1.10.0  HXTYPE_SVAL
1.10.0  1.10.0  1.10.0  HXTYPE_UCHAR
1.10.0  1.10.0  1.10.0  HXTYPE_UINT
1.10.0  1.10.0  1.10.0  HXTYPE_ULLONG
1.10.0  1.10.0  1.10.0  HXTYPE_ULONG
1.10.0  1.10.0  1.10.0  HXTYPE_USHORT
1.10.0  1.10.0  1.10.0  HXTYPE_VAL
1.10.0  1.10.0  1.10.0  HX_chomp
1.10.0  1.10.0  1.10.0  HX_copy_dir
1.10.0  1.10.0  1.10.0  HX_copy_file
1.10.0  1.10.0  1.10.0  HX_dirname
1.10.0  1.10.0  1.10.0  HX_dlclose
1.10.0  1.10.0  1.10.0  HX_dlerror
1.10.0  1.10.0  1.10.0  HX_dlopen
1.10.0  1.10.0  1.10.0  HX_dlsym
1.10.0  1.10.0  1.10.0  HX_dlsym<>
1.10.0  1.10.0  1.10.0  HX_getopt
1.10.0  1.10.0  1.10.0  HX_getopt_help
1.10.0  1.10.0  1.10.0  HX_getopt_usage
1.10.0  1.10.0  1.10.0  HX_irand
1.10.0  1.10.0  1.10.0  HX_memdup
1.10.0  1.10.0  1.10.0  HX_memdup<>
1.10.0  1.10.0  1.10.0  HX_rand
1.10.0  1.10.0  1.10.0  HX_rrmdir
1.10.0  1.10.0  1.10.0  HX_shconfig_free
1.10.0  1.10.0  1.10.0  HX_split5
1.10.0  1.10.0  1.10.0  HX_strbchr
1.10.0  1.10.0  1.10.0  HX_strclone
1.10.0  1.10.0  1.10.0  HX_strdup
1.10.0  1.10.0  1.10.0  HX_strlcat
1.10.0  1.10.0  1.10.0  HX_strlcpy
1.10.0  1.10.0  1.10.0  HX_strlncat
1.10.0  1.10.0  1.10.0  HX_strlower
1.10.0  1.10.0  1.10.0  HX_strltrim
1.10.0  1.10.0  1.10.0  HX_strrcspn
1.10.0  1.10.0  1.10.0  HX_strrev
1.10.0  1.10.0  1.10.0  HX_strrtrim
1.10.0  1.10.0  1.10.0  HX_strsep
1.10.0  1.10.0  1.10.0  HX_strsep2
1.10.0  1.10.0  1.10.0  HX_strupper
1.10.0  1.10.0  1.10.0  HX_zvecfree
1.10.0  1.10.0  1.10.0  HXdeque_del
1.10.0  1.10.0  1.10.0  HXdeque_find
1.10.0  1.10.0  1.10.0  HXdeque_free
1.10.0  1.10.0  1.10.0  HXdeque_get
1.10.0  1.10.0  1.10.0  HXdeque_init
1.10.0  1.10.0  1.10.0  HXdeque_move
1.10.0  1.10.0  1.10.0  HXdeque_pop
1.10.0  1.10.0  1.10.0  HXdeque_push
1.10.0  1.10.0  1.10.0  HXdeque_shift
1.10.0  1.10.0  1.10.0  HXdeque_to_vec
1.10.0  1.10.0  1.10.0  HXdeque_to_vec<>
1.10.0  1.10.0  1.10.0  HXdeque_unshift
1.10.0  1.10.0  1.10.0  SHCONF_ONE
======  ======  ======  ========================================


Struct reference
================

======  ======  ================================================
MinVer  FirstA
======  ======  ================================================
2.0     2.0     struct HXdeque_node.sptr
1.10.0  1.10.0  struct HXdeque_node
1.10.0  1.10.0  struct HXdeque
1.15    1.15    struct HXclist_head
1.15    1.15    struct HXlist_head
3.0     3.0     struct HXmap
3.0     3.0     struct HXmap_ops
3.0     3.0     struct HXmap_node
3.12    1.10.0  struct HXoptcb
3.12    1.10.0  struct HXoption
2.2     2.2     struct HXproc_ops
2.2     2.2     struct HXproc
======  ======  ================================================


Header reference
================

======  ===================================
MinVer  Name
======  ===================================
3.9     libHX/io.h
3.4     libHX/init.h
3.0     libHX/map.h
2.6     libHX/wx_helper.hpp
2.2     libHX/proc.h
2.0     libHX/ctype_helper.h
1.23    libHX/misc.h
1.23    libHX/defs.h
1.22    libHX/xml_helper.h
1.15    libHX/string.h
1.15    libHX/option.h
1.15    libHX/list.h
1.15    libHX/deque.h
======  ===================================
