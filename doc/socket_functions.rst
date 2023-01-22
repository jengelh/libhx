================
Socket functions
================

.. code-block:: c

	#include <libHX/socket.h>

	int HX_socket_from_env(const struct addrinfo *ai, const char *intf);
	int HX_sockaddr_is_local(const struct sockaddr *, socklen_t, unsigned int flags);
	int HX_ipaddr_is_local(const char *, unsigned int flags);

``HX_socket_from_env``
	The function looks up the current process's file descriptors for a
	socket that is listening and which matches the given addrinfo and
	(optionally) intf if the latter is not NULL``. Upon success, the fd
	number is returned, or -1 if no file descriptor matched. No errors are
	signalled.

``HX_sockaddr_is_local``
	Attempts to determine if the given socket address refers to a local
	address. This function may be helpful in determining whether a process
	should spend any time (or not) on compressing data before sending to a
	peer. The definition of "local" is generally dependent upon the network
	APIs. The ``flags`` parameter can contain ``AI_V4MAPPED`` if
	IPv4-mapped IPv6 addresses should be recognized. The function returns
	>0 if the address is considered local, 0 if not, and any other
	negative value for a system error that makes the result
	indeterminate.

``HX_ipaddr_is_local``
	Takes a text representation of an IPv6/IPv4 address and, after
	transformation, calls ``HX_sockaddr_is_local``.  ``flags`` and
	return value behave the same as that.
