#ifndef _LIBHX_SOCKET_H
#define _LIBHX_SOCKET_H 1

#ifdef _WIN32
#	include <ws2tcpip.h>
#else
#	include <netdb.h>
#	include <sys/socket.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern int HX_socket_from_env(const struct addrinfo *, const char *intf);
extern int HX_sockaddr_is_local(const struct sockaddr *, socklen_t, unsigned int flags);
extern int HX_ipaddr_is_local(const char *, unsigned int flags);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _LIBHX_SOCKET_H */
