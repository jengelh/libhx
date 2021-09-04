#ifndef _LIBHX_SOCKET_H
#define _LIBHX_SOCKET_H 1

#ifdef _WIN32
#	include <ws2tcpip.h>
#else
#	include <netdb.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern int HX_socket_from_env(const struct addrinfo *, const char *intf);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _LIBHX_SOCKET_H */
