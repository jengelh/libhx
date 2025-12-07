==============
Memory helpers
==============

``unique_tie``
==============

``unique_tie`` creates a proxy object for ``std::unique_ptr<T,D>`` instances to
interact with foreign functions that output a value through a ``T**`` pointer.

Normally, for functions which return their result through an argument pointer,
a temporary variable may be necessary when one wishes to use unique_ptr:

.. code-block: c++

	struct mydel { void operator()(void *x) const { free(x); } };

	unique_ptr<char[], mydel> u;
	char *x;
	bla_alloc(&x);
	u.reset(x);

With ``unique_tie``, this can be shortened to:

.. code-block: c++

	unique_ptr<char[], mydel> u;
	bla_alloc(&unique_tie(u));

This is similar to C++23's ``std::out_ptr`` and ``std::in_out_ptr``.
``unique_tie`` has subtle differences, though:

* Only usable to ``unique_ptr``, not ``shared_ptr`` or raw pointers.
* No implict conversions / No user-defined conversion operators.
* No ``void **`` conversion.
* Clearing is explicit, with ``~``.
* Address-taking is explicit in text, i.e. you have to type a ``&`` in source
  code. This is a deliberate choice for helping trivial text grepping for
  pointer-taking.
* There is higher memory use for when using a unique_ptr with custom deleter
  function, but the optimizer might just optimize it away anyway.

Repeated use of a variable with clearing inbetween works like so:

.. code-block: c++

	unique_ptr<char[], mydel> u;
	iconvxx("utf-8", "utf-16", &unique_tie(u), srctext1);
	printf("%s\n", u.get());
	iconvxx("utf-8", "utf-16", &~unique_tie(u), srctext2);
	printf("%s\n", u.get());
	iconvxx("utf-8", "utf-16", &~unique_tie(u), srctext3);
	printf("%s\n", u.get());

It is acceptable to employ/enforce ``&~`` in all uses – even the
first – of unique_tie in your project to guard against human error.
