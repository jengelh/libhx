============
Scope guards
============

``scope_exit``
==============

scope_exit creates an object that runs a predefined function when a scope ends.
This is useful for augmenting C APIs with something of a destructor without
wrapping the stuff in an explicit class of its own. For instance,

.. code-block:: c++

	#include <libHX/option.h>
	#include <libHX/scope.hpp>
	int main() {
		auto fa = HXformat_init();
		if (fa == nullptr)
			return 0;
		auto cleanup_fa = HX::make_scope_exit([&]() { HXformat_free(fa); });
		HXformat_add(fa, "foo", "bar", HXTYPE_STRING);
	}
