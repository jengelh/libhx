====
Maps
====

A map is a collection of key-value pairs. (Some languages, such as Perl, also
call them “associative array” or just “hash”, however, the underlying storage
mechanism may not be an array or a hash, however.) Each key is unique and has
an associated value. Keys can be any data desired; HXmap allows to specify your
own key and data handling functions so they can be strings, raw pointers, or
complex structures.

To access any map-related functions, ``#include <libHX/map.h>``.


Structural definition
=====================

The HXmap structure is a near-opaque type. Unlike the predecessor map
implementation struct HXbtree from libHX 2.x, the 3.x API exposes much less
fields.

.. code-block:: c

	struct HXmap {
	        unsigned int items, flags;
	};

``items``
	The number of items in the tree. This field tracks the number of items
	in the map and is used to report the number of elements to the user,
	and is updated whenever an element is inserted or removed from the map.
	The field must not be changed by the user.

``flags``
	The current behavior flags for the map. While implementation-private
	bits are exposed, only ``HXMAP_NOREPLACE`` is currently allowed to be
	(un)set by the user while a map exists.

For retrieving elements from a tree, some functions work with ``struct
HXmap_node``, which is defined as follows:

.. code-block:: c

	struct HXmap_node {
		union {
			void *key;
			const char *const skey;
		};
		union {
			void *data;
			char *sdata;
		};
	};

``key``
	The so-called primary key, which uniquely identifies an element (a
	key-value pair) in the map. The memory portions that make up the key
	must not be modified. (If the key changes, so does its hash value
	and/or position index, and without taking that into account, writing to
	the key directly is going to end up with an inconsistent state. To
	change the key, you will need to delete the element and reinsert it
	with its new key.)

``skey``
	A convenience type field for when the map's keys are C strings. It is
	useful for use with e.g. ``printf`` or other varargs function, which
	would otherwise require explicit and noisy casting of the ``void *key``
	member to ``const char *`` first.

``data``
	The data associated with the key.

``sdata``
	Convenience type field.


Map initialization
==================

During initialization, you specify the underlying storage type by selecting a
given constructor function. All further operations are done through the unified
HXmap API which uses a form of virtual calls internally.

Currently, there are two distinct map types in libHX. There are a handful of
selectable symbols, though. Abstract types are:

``HXMAPT_DEFAULT``
	No further preferences or guarantees; selects any data structure that the
	libHX maintainer deemed fast.

``HXMAPT_ORDERED``
	The map shall use a data structure that provides ordered traversal.

Specific types include:

``HXMAPT_HASH``
	Hash-based map – Amortized O(1) insertion, lookup and deletion;
	unordered.

``HXMAPT_RBTREE``
	Red-black binary tree – O(log(n)) insertion, lookup and deletion;
	ordered.

These can then be used with the initialization functions:

.. code-block:: c

	struct HXmap *HXmap_init(unsigned int type, unsigned int flags);
	struct HXmap *HXmap_init5(unsigned int type, unsigned int flags, const struct HXmap_ops *ops, size_t key_size, size_t data_size);

Both the *init* and *init5* variant creates a new map; the latter function
allows to specify the operations in detail as well as key and data size, which
may become necessary when using data sets which have their own way of being
managed. The flags parameter can contain any of the following:

``HXMAP_NONE``
	This is just a mnemonic for the value 0, indicating no flags.

``HXMAP_NOREPLACE``
	If a key already exists and another add operation is attempted, the
	key's associated value will be replaced by the new value. If this flag
	is absent, ``-EEXIST`` is returned. This flag is allowed to be
	subsequently changed by the user if so desired, using bit logic such as
	``map->flags &= ~HXMAP_NOREPLACE;``.

``HXMAP_SKEY``
	Notifies the constructor that keys will be C-style strings. The flag
	presets the ``k_compare`` operation to use ``strcmp``. In the flag's
	absence, direct value comparison will be used if the key size is
	specified as zero (e.g. with the ``HXhashmap_init4`` function call), or
	``memcmp`` if the key size is non-zero.

``HXMAP_CKEY``
	Instructs the map to make copies of keys when they are added to the
	map. This is required when the buffer holding the key changes or goes
	out of scope. The flag presets the ``k_clone`` and ``k_free``
	operations to ``HX_memdup`` and ``free``, and as such, the ``key_size``
	parameter must not be zero. If however, ``HXMAP_SKEY`` is also
	specified, ``HX_strdup`` and ``free`` will be used and ``key_size``
	must be ``zero``.

``HXMAP_SDATA``
	Notifies the constructor that data will be C-style strings. This sets
	up the ``d_clone`` and ``d_free`` operations.

``HXMAP_CDATA``
	Instructs the map to make copies of the data when new entries are added
	to the map. This is required when the buffer holding the data either
	goes out of scope, or you want to keep the original contents instead of
	just a pointer.

``HXMAP_SCKEY``
	Mnemonic for the combination of ``HXMAP_SKEY | HXMAP_CKEY``.

``HXMAP_SCDATA``
	Mnemonic for the combination of ``HXMAP_SDATA | HXMAP_SDATA``.

``HXMAP_SINGULAR``
	Specifies that the “map” is only used as a set, i.e. it does not store
	any values, only keys. Henceforth, the value argument to ``HXmap_add``
	must always be ``NULL``.


Flag combinations
=================

This subsection highlights the way ``HXMAP_SKEY`` interacts with ``HXMAP_CKEY``
and the key size. The copy semantics are the same for ``HXMAP_SDATA`` and
``HXMAP_CDATA``.

HXMAP_SKEY unset, HXMAP_CKEY unset
----------------------------------

The ``key_size`` parameter at the time of map construction is ignored. The
pointer value of the key parameter for the ``HXmap_add`` call is directly
stored in the tree, and this is the key that uniquely identifies the map entry
and which is used for comparisons. This may be used if you intend to directly
map pointer values.

.. code-block:: c

	static struct something *x = ..., *y = ...;
	HXmap_add(map, &x[0], "foo");
	HXmap_add(map, &x[1], "bar");

HXMAP_SKEY set, HXMAP_CKEY unset
--------------------------------

The ``key_size`` parameter at the time of map construction is ignored. The
pointer value of the key parameter for the HXmap_add call is directly stored in
the tree, but it is the C string pointed to by the key parameter that serves as
the key.

HXMAP_SKEY set, HXMAP_CKEY set
------------------------------

The ``key_size`` parameter at the time of map construction is ignored. The
string pointed to by the key parameter will be duplicated, and the resulting
pointer will be stored in the tree. Again, it is the pointed-to string that is
the key.

HXMAP_SKEY unset, HXMAP_CKEY set
--------------------------------

The memory block pointed to by the key parameter will be duplicated. The
``key_size`` parameter must be non-zero for this to successfully work.

With separate ops
-----------------

However, when a custom ``struct HXmap_ops`` is provided in the call to
``HXmap_init5``, any of these semantics can be overridden. Particularly, since
your own ops can practically ignore ``key_size``, it could be set to any value.


Key-data operations
===================

The ``HXMAP_SKEY/CKEY/SDATA/CDATA`` flags are generally sufficient to set up
common maps where keys and/or data are C strings or simple binary data where
``memdup``/``memcmp`` is enough. Where the provided mechanisms are not cutting
it, an extra ``HXmap_ops`` structure with functions specialized in handling the
keys and/or data has to be used as an argument to the initialization function
call.

.. code-block:: c

	struct HXmap_ops {
		int (*k_compare)(const void *, const void *, size_t);
		void *(*k_clone)(const void *, size_t);
		void (*k_free)(void *);
		void *(*d_clone)(const void *, size_t);
		void (*d_free)(void *);
		unsigned long (*k_hash)(const void *, size_t);
	};

``k_compare``
	Function to compare two keys. The return value is the same as that of
	``memcmp`` or ``strcmp``: negative values indicate that the first key
	is “less than” the second, zero indicates that both keys are equal, and
	positive values indicate that the first key is “greater than” the
	second. The ``size`` argument in third position is provided so that
	``memcmp``, which wants a size parameter, can directly be used without
	having to write an own function. (This also means strcmp can't be
	directly plugged in due to a function signature mismatch.)

``k_clone``
	Function that will clone (duplicate) a key. This is used for keys that
	will be added to the tree, and potentially also for state-keeping
	during traversal of the map. It is valid that this clone function
	simply returns the value of the pointer it was actually passed; this is
	used by default for maps without ``HXMAP_CKEY`` for example.

``k_free``
	Function to free a key. In most cases it defaults to ``free``(3), but
	in case you are using complex structs, more cleanup may be needed.

``d_clone``
	Same idea as ``k_clone``, but for data.

``d_free``
	Same idea as ``k_free``, but for data.

``k_hash``
	Specifies an alternate hash function. Only to be used with hash-based
	maps. Hashmaps default to using the DJB2 string hash function when
	``HXMAP_SKEY`` is given, or otherwise the Jenkins' lookup3 hash
	function.

libHX exports two hash functions that you can select for ``struct HXmap_ops``'s
``k_hash`` if the default for a given flag combination is not to your liking.

``HXhash_jlookup3``
	Bob Jenkins's lookup3 hash.

``HXhash_djb2``
	DJB2 string hash.


Map operations
==============

.. code-block:: c

	int HXmap_add(struct HXmap *, const void *key, const void *value);
	const struct HXmap_node *HXmap_find(const struct HXmap *, const void *key);
	void *HXmap_get(const struct HXmap *, const void *key);
	void *HXmap_del(struct HXmap *, const void *key);
	void HXmap_free(struct HXmap *);
	struct HXmap_node *HXmap_keysvalues(const struct HXmap *);

``HXmap_add``
	Adds a new node to the tree using the given key and data. When an
	element is in the map, the key may not be modified, as doing so could
	possibly invalidate the internal location of the element, or its
	ordering with respect to other elements. If you need to change the key,
	you will have to delete the element from the tree and re-insert it. On
	error, -errno will be returned.

	When ``HXMAP_SINGULAR`` is in effect, value must be ``NULL``, else
	``-EINVAL`` is returned.

``HXmap_find``
	Finds the node for the given key. The key can be read from the node
	using ``node->key`` or ``node->skey`` (convenience alias for key, but
	with a type of ``const char *``), and the data by using ``node->data``
	or ``node->sdata``.

``HXmap_get``
	Get is a find operation directly returning ``node->data`` instead of
	the node itself. Since ``HXmap_get`` may legitimately return ``NULL``
	if ``NULL`` was stored in the tree as the data for a given key, only
	``errno`` will really tell whether the node was found or not; in the
	latter case, ``errno`` is set to ``ENOENT``.

``HXmap_del``
	Removes an element from the map and returns the data value that was
	associated with it. When an error occurred, or the element was not
	found, ``NULL`` is returned. Because ``NULL`` can be a valid data
	value, ``errno`` can be checked for non-zero. ``errno`` will be
	``-ENOENT`` if the element was not found, or zero when everything was
	ok.

``HXmap_free``
	The function will delete all elements in the map and free memory it
	holds.

``HXmap_keysvalues``
	Returns all key-value-pairs in an array of the size as many items were
	in the map (map->items) at the time it was called. The memory must be
	freed using ``free``(3) when it is no longer needed. The order elements
	in the array follows the traverser notes (see below), unless otherwise
	specified.


Map traversal
=============

.. code-block:: c

	struct HXmap_trav *HXmap_travinit(const struct HXmap *);
	const struct HXmap_node *HXmap_traverse(struct HXmap_trav *iterator);
	void HXmap_travfree(struct HXmap_trav *iterator);
	void HXmap_qfe(const struct HXmap *, bool (*fn)(const struct HXmap_node *, void *arg), void *arg);

``HXmap_travinit``
	Initializes a traverser (a.k.a. iterator) for the map, and returns a
	pointer to it. ``NULL`` will be returned in case of an error, such as
	memory allocation failure. Traversers are returned even if the map has
	zero elements.

``HXmap_traverse``
	Returns a pointer to a ``struct HXmap_node`` for the next element /
	key-value pair from the map, or ``NULL`` if there are no more entries.

``HXmap_travfree``
	Releases the memory associated with a traverser.

``HXmap_qfe``
	The “quick foreach”. Iterates over all map elements in the fastest
	possible manner, but has the restriction that no modifications to the
	map are allowed. Furthermore, a separate function to handle each
	visited node, is required. (Hence this is also called “closed
	traversal”, because one cannot access the stack frame of the original
	function which called ``HXmap_qfe``.) The user-defined function returns
	a bool which indicates whether traversal shall continue or not.

Flags for ``HXmap_travinit``:

``HXMAP_NOFLAGS``
	A mnemonic for no flags, and is defined to 0.

``HXMAP_DTRAV``
	Enable support for deletion during traversal. As it can make traversal
	slower, it needs to be explicitly specified for cases where it is
	needed, to not penalize cases where it is not.

WARNING: Modifying the map while a traverser is active is
implementation-specific behavior! libHX generally ensures that there will be no
undefined behavior (e.g. crashes), but there is no guarantee that elements
will be returned exactly once. There are fundamental cases that one should be
aware of:

* An element is inserted before where the traverser is currently positioned at.
  The element may not be returned in subsequent calls to ``HXmap_traverse`` on
  an already-active traverser.

* Insertion or deletion may cause internal data structure to re-layout.

  * Traversers of ordered data structures may choose to rebuild
    their state.

  * Traversers of unordered data structures would run risk to
    return more than once, or not at all.

Descriptions for different map types follow.

:Hashmaps:
	On ```HXmap_add`, an element may be inserted in a position that is
	before where the traverser is currently positioned. Such elements will
	not be returned in the remaining calls to ``HXmap_traverse``. The
	insertion or deletion of an element may cause the internal data
	structure to re-layout itself. When this happens, the traverser will
	stop, so as to not return entries twice.

:Binary trees:
	Elements may be added before the traverser's position. These elements
	will not be returned in subsequent traversion calls. If the data
	structure changes as a result of an addition or deletion, the traverser
	will rebuild its state and continue traversal transparently. Because
	elements in a binary tree are ordered, that is, element positions may
	not change with respect to another when the tree is rebalanced, there
	is no risk of returning entries more than once. Nor will elements that
	are sorted after the current traverser's position not be returned
	(= they will be returned, because they cannot get reordered to before
	the traverser like in a hash map). The HX rbtree implementation also
	has proper handling for when the node which is currently visiting is
	deleted.


RB-tree Limitations
===================

The implementation has a theoretical minimum on the maximum number of nodes,
2^{24}=16{,}777{,}216. A worst-case tree with this many elements already has a
height of 48 (RBT_MAXDEP), which is the maximum height currently supported. The
larger the height is that HXrbtree is supposed to handle, the more memory
(linear increase) it needs. All functions that build or keep a path reserve
memory for RBT_MAXDEP nodes; on x86_64, this is 9 bytes per <node, direction>
pair, amounting to 432 bytes for path tracking alone. It may not sound like a
lot to many, but given that kernel developers try to limit their stack usage to
some 4096 bytes is impressive alone.


Examples
========

Case-insensitive ordering
-------------------------

The correct way:

.. code-block:: c

	static int my_strcasecmp(const void *a, const void *b, size_t z) {
		return strcasecmp(a, b);
	}
	static const struct HXmap_ops icase = {
		.k_compare = my_strcasecmp,
	};
	HXmap_init5(HXMAPT_RBTREE, HXMAP_SKEY, &icase, 0, dsize);

A hackish way (which wholly depends on the C implementation and use of extra
safeguards is a must):

.. code-block:: c

	static const struct HXmap_ops icase = {
		.k_compare = (void *)strcasecmp,
	};
	BUILD_BUG_ON(sizeof(DEMOTE_TO_PTR(strcasecmp)) > sizeof(void *));
	BUILD_BUG_ON(sizeof(DEMOTE_TO_PTR(strcasecmp)) > sizeof(icase.k_compare));
	HXmap_init5(HXMAPT_RBTREE, HXMAP_SKEY, &icase, 0, dsize);


Reverse sorting order
---------------------

Any function that behaves like strcmp can be used. It merely has to return
negative when ``a<b``, zero on ``a==b``, and positive non-zero when ``a>b``.

.. code-block:: c

	static int strcmp_rev(const void *a, const void *b, size_t z)
	{
		/* z is provided for cases when things are raw memory blocks. */
		return strcmp(b, a);
	}

	static const struct HXmap_ops rev = {
		.k_compare = strcmp_rev,
	};
	HXmap_init5(HXMAPT_RBTREE, HXMAP_SKEY, &rev, 0, dsize);


Keys with non-unique data
-------------------------

Keys can actually store non-unique data, as long as this extra fields does not
actually contribute to the logical key — the parts that do uniquely identify
it. In the following example, the notes member may be part of struct package,
which is the key as far as HXmap is concerned, but still, only the name and
versions are used to identify it.

.. code-block:: c

	struct package {
		      char *name;
		      unsigned int major_version;
		      unsigned int minor_version;
		      char notes[64];
	};

	static int package_cmp(const void *a, const void *b)
	{
		const struct package *p = a, *q = b;
		int ret;
		ret = strcmp(p->name, q->name);
		if (ret != 0)
			return ret;
		ret = p->major_version - q->major_version;
		if (ret != 0)
			return ret;
		ret = p->minor_version - q->minor_version;
		if (ret != 0)
			return ret;
		return 0;
	}

	static const struct HXmap_ops package_ops = {
		.k_compare = package_cmp,
	};

	HXmap_init5(HXMAPT_RBTREE, flags, &package_ops,
		sizeof(struct package), dsize);
