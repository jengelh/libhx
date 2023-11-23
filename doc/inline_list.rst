=========================
Inline doubly-linked list
=========================

Classical linked-list implementations, such as HXdeque, either store the actual
data within a node, or indirectly through a pointer, but the “inline
doubly-linked list” instead does it reverse and has the list head within the
data structure.

A classic linked-list implementations with direct/indirect data blocks may look
like so:

.. code-block:: c

	struct package_desc {
		char *package_name;
		int version;

	};

	struct classic_direct_node {
		struct classic_direct_node *next, *prev;
		struct package_desc direct_data;
	};
	struct classic_indirect_node {
		struct classic_indirect_node *next, *prev;
		void *indirect_data;
	};

Whereas in an inline list, the list head (next,prev pointers) inlined into the data
block:

.. code-block:: c

	struct package_desc {
		struct HXlist_head list;
		char *package_name;
		int version;
	};

At first glance, an inline list does not look much different from ``struct
classic_direct_data``, it is mostly a viewpoint decision which struct is in the
foreground.


Synopsis
========

.. code-block:: c

	#include <libHX/list.h>

	struct HXlist_head {struct HXlist_head
		/* All fields considered private */
	};

	HXLIST_HEAD_INIT(name);
	HXLIST_HEAD(name);
	void HXlist_init(struct HXlist_head *list);
	void HXlist_add(struct HXlist_head *list, struct HXlist_head *elem);
	void HXlist_add_tail(struct HXlist_head *list, struct HXlist_head *elem);
	void HXlist_del(struct HXlist_head *element);
	bool HXlist_empty(const struct HXlist_head *list);

``HXLIST_HEAD_INIT``
	This macro expands to the static initializer for a list head.

``HXLIST_HEAD``
	This macro expands to the definition of a list head (i.e. ``struct
	HXlist_head name = HXLIST_HEAD_INIT;``).

``HXlist_init``
	Initializes the list head. This function is generally used when the
	list head is on the heap where the static initializer cannot be used.

``HXlist_add``
	Adds ``elem`` to the front of the list.

``HXlist_add_tail``
	Adds ``elem`` to the end of the list.

``HXlist_del``
	Deletes the given element from the list.

``HXlist_empty``
	Tests whether the list is empty. (Note: For clists, you could also use
	``clist->items == 0``).


Traversal
=========

Traversal is implemented using macros that expand to ``for()`` statements which
can syntactically be used like them, i.e. curly braces may be omitted if only
a single statement is in the body of the loop.

The head parameter specifies the list head (``struct HXlist_head``), ``pos``
specifies an iterator, also of type ``struct HXlist_head``. Lists can either be
traversed in forward direction, or, using the ``*_rev`` variants, in reverse
direction. The ``*_safe`` variants use a temporary ``n`` to hold the next
object in the list, which is needed when ``pos`` itself is going to be
inaccessible at the end of the block, through, for example, freeing its
encompassing object.

.. code-block:: c

	HXlist_for_each(pos, head)
	HXlist_for_each_rev(pos, head)
	HXlist_for_each_safe(pos, n, head)
	HXlist_for_each_rev_safe(pos, n, head)

``HXlist_for_each``
	Forward iteration over the list heads.

``HXlist_for_each_rev``
	Reverse iteration over the list heads.

``HXlist_for_each_safe``
	Forward iteration over the list heads that is safe against freeing pos.

``HXlist_for_each_rev_safe``
	Reverse iteration over the list heads that is safe against freeing pos.

The ``*_entry`` variants use an iterator ``pos`` of the type of the
encompassing object (e.g. ``struct item`` in below's example), so that the
manual ``HXlist_entry`` invocation is not needed. ``member`` is the name of the
list structure embedded into the item.

.. code-block:: c

	HXlist_for_each_entry(pos, head, member)HXlist_for_each_entry
	HXlist_for_each_entry_rev(pos, head, member)HXlist_for_each_entry_rev
	HXlist_for_each_entry_safe(pos, n, head, member)HXlist_for_each_entry_safe

``HXlist_for_each_entry``
	Forward iteration over the list elements.

``HXlist_for_each_entry_rev``
	Reverse iteration over the list elements.

``HXlist_for_each_entry_safe``
	Forward iteration over the list elements that is safe against freeing
	``pos``.


Examples
========

.. code-block:: c

	struct item {
		struct HXlist_head anchor;
		char name[32];
	};

	struct HXlist_head *e;
	struct item *i, *j;
	HXLIST_HEAD(list);

	i = malloc(sizeof(*i));
	HXlist_init(&e->anchor);
	strcpy(i->name, "foo");
	HXlist_add_tail(&list, &i->anchor);

	i = malloc(sizeof(*i));
	HXlist_init(&e->anchor);
	strcpy(i->name, "bar");
	HXlist_add_tail(&list, &i->anchor);

	HXlist_for_each(e, &list) {
		i = HXlist_entry(e, typeof(*i), anchor);
		printf("e=%p i=%p name=%s\n", e, i, i->name);
	}

	HXlist_for_each_entry(i, &list, anchor)
		printf("i=%p name=%s\n", i, i->name);

	HXlist_for_each_entry_rev(i, &list, anchor)
		printf("i=%p name=%s\n", i, i->name);

	HXlist_for_each_entry_safe(i, j, &list, anchor) {
		printf("i=%p name=%s\n", i, i->name);
		free(i);
	}


When to use HXdeque/HXlist
==========================

The choice whether to use HXdeque or HXlist/HXclist depends on whether one
wants the list head handling on the developer or on the library. Especially for
“atomic” and “small” data, it might be easier to just let HXdeque do the
management. Compare the following two code examples to store strings in a
HXdeque:

.. code-block:: c

	int main(int argc, char **argv)
	{
		struct HXdeque *dq = HXdeque_init();
		while (--argc)
			 HXdeque_push(dq, ++argv);
		return 0;
	}

...and to store strings in a HXlist:

.. code-block:: c

	struct element {
		struct HXlist_head list;
		char *data;
	};

	int main(int main, char **argv)
	{
		HXLIST_HEAD(lh);
		while (--argc) {
			struct element *e = malloc(sizeof(*e));
			e->data = *++argv;
			HXlist_init(&e->list);
			HXlist_add_tail(&e->list);
		}
		return 0;
	}

These examples assume that ``argv`` is persistent, which, for the sample, is
true.

With HXlist, one needs to have a struct with a ``HXlist_head`` in it, and if
one does not already have such a struct, e.g. by means of wanting to store more
than just one value, one will need to create it first, as shown, and this may
lead to an expansion of code.

This however does not mean that HXlist is the better solution over HXdeque for
data already available in a struct. As each struct has a ``list_head`` that is
unique to the node, it is not possible to share this data. Trying to add a
HXlist_head to another list is not going to end well, while HXdeque has no
problem with this as list heads are detached from the actual data in HXdeque.

Data can be added multiple times in a HXdeque without ill effects:

.. code-block:: c

	struct point p = {15, 30};
	HXdeque_push(dq, &p);
	HXdeque_push(dq, &p);

To support this, an extra allocation is needed on the other hand. In a HXlist,
to store *n* elements of compound data (e.g. ``struct point``), *n* allocations
are needed, assuming the list head is a stack object, and the points are not.
HXdeque will need at least *2n+1* allocations, *n* for the nodes, *n* for the
points and another for the head.
