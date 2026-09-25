================
Socket functions
================

.. code-block:: c

	#include <libHX/socket.h>

	int HX_addrport_split(const char *spec, char *host, size_t hsize, uint16_t *port);
	int HX_inet_connect(const char *host, uint16_t port, unsigned int oflags);
	int HX_inet_listen(const char *host, uint16_t port);
	int HX_local_listen(const char *path);
	int HX_socket_from_env(const struct addrinfo *ai, const char *intf);
	int HX_sockaddr_is_local(const struct sockaddr *, socklen_t, unsigned int flags);
	int HX_ipaddr_is_local(const char *, unsigned int flags);

``HX_addrport_split``
	Splits a host specification like ``[fe80::1]:80`` or ``127.0.0.1:80``
	into a host and port part. ``host`` is a buffer of size ``hsize`` that
	will be filled with the host portion. The port output parameter is only
	written to when not NULL. If both a host and a port parts are detected,
	the return value will be 2. If only a host part is detected, the return
	value will be 1 and the port output parameter will not be written to at
	all. Therefore, it makes sense to initialize the port variable to
	something prior to calling HX_addrport_split. Upon error, a negative
	errno value is returned.

``HX_inet_connect``
	The function first resolves the specified host or IPv6/IPv4 address
	(must not be enclosed in square brackets), and then attempts to connect
	to one of the addresses. The order of evaluation is dependent upon the
	system defaults. (It may choose whatever protocol is offered by the
	system.) ``oflags`` is a bitset which may contain ``O_NONBLOCK``, else
	must be 0. Upon success, a socket file descriptor is returned. Upon
	failure, a negative errno code is returned. The socket will have
	``SOCK_CLOEXEC`` set by default if the platform supports it.

``HX_inet_listen``
	The function first resolves ``host`` using ``getaddrinfo()` with
	``AI_PASSIVE``, then using ``HX_socket_from_env`` looks in the
	environment for a matching socket to pick up, and otherwise uses the
	first result from getaddrinfo to create a new socket. Upon error, a
	negative errno value is returned. The socket will have ``SOCK_CLOEXEC``
	set by default if the platform supports it.

``HX_local_listen``
	The function creates a local system-specific socket. Using
	``HX_socket_from_env``, it will attempt to pick up a matching socket
	from the environment, and otherwise create a new socket. Upon error, a
	negative errno value is returned. The socket will have ``SOCK_CLOEXEC``
	set by default if the platform supports it.

``HX_socket_from_env``
	The function looks up the current process's file descriptors for a
	socket that is listening and which matches the given addrinfo and
	(optionally) intf if the latter is not NULL``. Upon success, the fd
	number is returned, or -1 if no file descriptor matched. No errors are
	signalled. Before this function returns a file descriptor, it sets
	``SOCK_CLOEXEC``.

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

Examples
--------

.. code-block:: c

	char host[256];
	uint16_t port = 443;
	/* port won't be updated */
	HX_addrport_split("example.de", host, sizeof(host), &port);
	/* port will be updated */
	HX_addrport_split("example.de:80", host, sizeof(host), &port);
