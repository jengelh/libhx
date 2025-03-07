==============
Helper headers
==============

ctype helpers
=============

Functions from the ``<ctype.h>`` header, including, but not limited to,
``isalpha``, ``tolower``, and so forth, are defined to take an ``int`` as first
argument. C strings consist of ``char``s, which may either be ``signed`` or
``unsigned``. It does not matter for *characters_* but since ``char``s are
implicitly convertible to *numbers*, one needs to explicitly cast chars to
unsigned before feeding them to ``isalpha``.

.. code-block:: c

	/* “hyvää yötä”, UTF-8 encoded */

	const char h[] = {'h', 'y', 'v', 0xc3, 0xa4, 0xc3, 0xa4, ' ',
			  'y', 0xc3, 0xb6, 't', 0xc3, 0xa4};

libHX's ctype_helper.h therefore provides wrappers with a different function
signature that does the intended test. It does so by always taking ``unsigned
char``. The implication is that EOF cannot be passed to ``HX_isalpha`` — not
that there is a good reason to do so in the first place.

.. code-block:: c

	#include <libHX/ctype_helper.h>

	bool HX_isalnum(unsigned char c);
	bool HX_isalpha(unsigned char c);
	bool HX_isascii(unsigned char c);
	bool HX_isdigit(unsigned char c);
	bool HX_islower(unsigned char c);
	bool HX_isprint(unsigned char c);
	bool HX_isspace(unsigned char c);
	bool HX_isupper(unsigned char c);
	bool HX_isxdigit(unsigned char c);
	unsigned char HX_tolower(unsigned char c);
	unsigned char HX_toupper(unsigned char c);

The ``HX_is*`` functions also differ from ctype's in that they return ``bool``
instead of ``int``. Not all functions from ``ctype.h`` are present either;
``isascii``, ``isblank``, ``iscntrl``, ``isgraph``, ``ispunct`` and
``isxdigit`` have been omitted as the author has never needed them to this
date.


libxml2 helpers
===============

libxml2 defines a type called ``xmlChar`` for use within its strings.
``xmlChar`` is typedef'ed to ``unsigned char`` by libxml2, causing compiler warnings related to
differing signedness whenever interacting with strings from the
outside world, which are usually just a pointer to char. Because
casting is a real chore, ``libxml_helper.h`` will do it by
providing some wrappers with better argument types.

.. code-block:: c

	#include <libHX/libxml_helper.h>

	int xml_strcmp(const xmlChar *a, const char *b);
	int xml_strcasecmp(const xmlChar *a, const char *b);

	char *xml_getprop(xmlNode *node, const char *attr);
	char *xml_getnsprop(xmlNode *node, const char *nsprefix, const char *attr);
	xmlAttr *xml_newprop(xmlNode *node, const char *attr);
	xmlNode *xml_newnode(xmlNode *parent, const char *name, const char *value);
	xmlAttr *xml_setprop(xmlNode *node, const char *name, const char *value);

The functions map to ``strcmp``, ``strcasecmp``, ``xmlGetProp``,
``xmlNewPropxmlNewProp``, ``xmlNewTextNodexmlNewTextNode`` and
``xmlSetPropxmlSetProp``, respectively.

``xml_getnsprop`` works similar to ``xmlGetNsProp``, but instead of taking a
namespace URI, it does a lookup by namespace prefix. The argument order is also
different compared to ``xmlGetNsProp``.


wxWidgets
=========

.. code-block:: c++

	#include <libHX/wx_helper.hpp>

Shortcut macros
---------------

``wxACV``
	Expands to ``wxALIGN_CENTER_VERTICAL``.

``wxCDF``
	Expands to a set of common dialog flags for wxDialogs, which includes
	``wxDEFAULT_FRAME_STYLE`` and a flag such that the dialog does not
	create a new window in the task bar (``wxFRAME_NO_TASKBAR``).

``wxDPOS``
	Expands to ``wxDefaultPosition``.

``wxDSIZE``
	Expands to ``wxDefaultSize``.

``wxDSPAN``
	Expands to ``wxDefaultSpan``.

String conversion
-----------------

.. code-block:: c++

	wxString wxfu8(const char *);
	wxString wxfv8(const char *);
	const char *wxtu8(const wxString &);

``wxfu8``
	Converts a UTF-8 string to a ``wxString`` object.

``wxfv8``
	Converts a UTF-8 string to an entity usable by ``wxPrintf``.

``wxtu8``
	Converts a wxString to a pointer to char usable by ``printf``. Note
	that the validity of the pointer is very limited and usually does not
	extend beyond the statement in which it is used. Hence, storing the
	pointer in a variable (``const char *p = wxtu8(s);``) will make ``p``
	dangling as soon as the assignment has been completed.
