=================================
Counted inline doubly-linked list
=================================

clist is the inline doubly-linked list cousin of the inline doubly-linked list,
extended by a counter to retrieve the number of elements in the list in O(1)
time. This is also why all operations always require the list head. For
traversal of clists, use the corresponding HXlist macros.

Synopsis
========

.. code-block:: c

	#include <libHX/list.h>

	struct HXclist_head {
		/* public readonly: */
		unsigned int items;
		/* Undocumented fields are considered “private” */
	};

	HXCLIST_HEAD_INIT(name);
	HXCLIST_HEAD(name);
	void HXclist_init(struct HXclist_head *head);
	void HXclist_unshift(struct HXclist_head *head, struct HXlist_head *new_node);
	void HXclist_push(struct HXclist_head *head, struct HXlist_head *new_node);
	type HXclist_pop(struct HXclist_head *head, type, member);
	type HXclist_shift(struct HXclist_head *head, type, member);
	void HXclist_del(struct HXclist_head *head, struct HXlist_chead *node);

``HXCLIST_HEAD_INIT``
	Macro that expands to the static initializer for a clist.

``HXCLIST_HEAD``
	Macro that expands to the definition of a clist head, with
	initialization.

``HXclist_init``
	Initializes a clist. This function is generally used when the head has
	been allocated from the heap.

``HXclist_unshift``
	Adds the node to the front of the list.

``HXclist_push``
	Adds the node to the end of the list.

``HXclist_pop``
	Removes the last node in the list and returns it.

``HXclist_shift``
	Removes the first node in the list and returns it.

``HXclist_del``
	Deletes the node from the list.

The list count in the clist head is updated whenever a modification is done on
the clist through these functions.
