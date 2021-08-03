==================
Doubly-linked list
==================

HXdeque is a data structure for a doubly-linked non-circular
``NULL``-sentineled list. Despite being named a deque, which is short for
double-ended queue, and which may be implemented using an array, HXdeque is in
fact using a linked list to provide its deque functionality. Furthermore, a
dedicated root structure and decidated node structures with indirect data
referencing are used.


Structural definition
=====================

.. code-block:: c

	#include <libHX/deque.h>

	struct HXdeque {
		struct HXdeque_node *first, *last;
		unsigned int items;
		void *ptr;
	};

	struct HXdeque_node {
		struct HXdeque_node *next, *prev;
		struct HXdeque *parent;
		void *ptr;
	};

The ``ptr`` member in ``struct HXdeque`` provides room for an arbitrary custom
user-supplied pointer. items will reflect the number of elements in the list,
and must not be modified. ``first`` and ``last`` provide entrypoints to the
list's ends.

``ptr`` within ``struct HXdeque_node`` is the pointer to the user's data. It
may be modified and used at will by the user. See example section.


Constructor, destructors
========================

.. code-block:: c

	struct HXdeque *HXdeque_init(void);
	void HXdeque_free(struct HXdeque *dq);
	void HXdeque_genocide(struct HXdeque *dq);
	void HXdeque_genocide2(struct HXdeque *dq, void (*xfree)(void *));
	void **HXdeque_to_vec(struct HXdeque *dq, unsigned int *num);

To allocate a new empty list, use ``HXdeque_init``. ``HXdeque_free`` will free
the list (including all nodes owned by the list), but not the data pointers.

``HXdeque_genocide`` is a variant that will not only destroy the list, but also
calls a freeing function ``free``() on all stored data pointers. This puts a
number of restrictions on the characteristics of the list: all data pointers
must have been obtained with ``malloc``, ``calloc`` or ``realloc`` before, and
no data pointer must exist twice in the list. The function is more efficient
than an open-coded loop over all nodes calling ``HXdeque_del``.

A generic variant is available with ``HXdeque_genocide2``, which takes a
pointer to an appropriate freeing function. ``HXdeque_genocide`` is thus
equivalent to ``HXdeque_genocide2(dq, free)``.

To convert a linked list to a ``NULL``-terminated array, ``HXdeque_to_vec`` can
be used. If ``num`` is not ``NULL``, the number of elements excluding the
``NULL`` sentinel, is stored in ``*num``.


Addition and removal
====================

.. code-block:: c

	struct HXdeque_node *HXdeque_push(struct HXdeque *dq, void *ptr);
	struct HXdeque_node *HXdeque_unshift(struct HXdeque *dq, void *ptr);
	void *HXdeque_pop(struct HXdeque *dq);
	void *HXdeque_shift(struct HXdeque *dq);
	struct HXdeque *HXdeque_move(struct HXdeque_node *target, struct HXdeque_node *node);
	void *HXdeque_del(struct HXdeque_node *node);

``HXdeque_push`` and ``HXdeque_unshift`` add the data item in a new node at the
end (“push”) or as the new first element (“unshift” as Perl calls it),
respectively. The functions will return the new node on success, or ``NULL`` on
failure and errno will be set. The node is owned by the list.

``HXdeque_pop`` and ``HXdeque_shift`` remove the last (“pop”) or first
(“shift”) node, respectively, and return the data pointer that was stored in
the data.

``HXdeque_move`` will unlink a node from its list, and reinsert it after the
given target node, which may be in a different list.

Deleting a node is accomplished by calling ``HXdeque_del`` on it. The data
pointer stored in the node is not freed, but returned.


Iteration
=========

Iterating over a HXdeque linked list is done manually and without additional
overhead of function calls:

.. code-block:: c

	const struct HXdeque_node *node;
	for (node = dq->first; node != NULL; node = node->next)
		do_something(node->ptr);


Searching
=========

.. code-block:: c

	struct HXdeque_node *HXdeque_find(struct HXdeque *dq, const void *ptr);
	void *HXdeque_get(struct HXdeque *dq, void *ptr);

``HXdeque_find`` searches for the node which contains ``ptr``, and does so by
beginning at the start of the list. If no node is found, ``NULL`` is returned.
If a pointer is more than once in the list, any node may be returned.

``HXdeque_get`` will further return the data pointer stored in the node —
however, since that is just what the ptr argument is, the function practically
only checks for existence of ``ptr`` in the list.


Example: Using HXdeque to store and sort a list
===============================================

.. code-block:: c

	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <libHX/defs.h>
	#include <libHX/deque.h>
	#include <libHX/string.h>
	#include <pwd.h>

	int main(void)
	{
		struct HXdeque *dq = HXdeque_init();
		struct passwd *pw;
		unsigned int elem;
		char **users;

		setpwent();
		while ((pw = getpwent()) != NULL)
			HXdeque_push(dq, HX_strdup(pw->pw_name));
		endpwent();

		users = reinterpret_cast(char **, HXdeque_to_vec(dq, &elem));
		HXdeque_free(dq);

		qsort(users, elem, sizeof(*users), static_cast(void *, strcmp));
		return 0;
	}

In this example, all usernames are obtained from NSS, and put into a list.
``HX_strdup`` is used, because ``getpwent`` will overwrite the buffer it uses
to store its results. The list is then converted to an array, and the list is
freed (because it is not need it anymore). ``HXdeque_genocide`` must not be
used here, because it would free all the data pointers (strings here) that were
just inserted into the list. Finally, the list is sorted. Because ``strcmp``
takes two ``const char *`` arguments, but qsort mandates a function taking two
``const void *``, a cast can be used to silence the compiler. This only works
if and when ``char *`` pointers have the same size as ``void *``, and because
we know that the array consists of ``char *`` pointers, and not somehting else;
therefore, ``strcmp`` will work.
