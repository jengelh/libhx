================
Socket functions
================

.. code-block:: c

	#include <libHX/socket.h>

	int HX_socket_from_env(const struct addrinfo *ai, const char *intf);

``HX_socket_from_env``
	The function looks up the current process's file descriptors for a
	socket that is listening and which matches the given addrinfo and
	(optionally) intf if the latter is not NULL``. Upon success, the fd
	number is returned, or -1 if no file descriptor matched. No errors are
	signalled.
