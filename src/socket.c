/*
 *	Socket-related functions
 *	Copyright Jan Engelhardt, 2021
 *
 *	This file is part of libHX. libHX is free software; you can
 *	redistribute it and/or modify it under the terms of the GNU Lesser
 *	General Public License as published by the Free Software Foundation;
 *	either version 2.1 or (at your option) any later version.
 */
#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#	include <ws2tcpip.h>
#else
#	include <sys/socket.h>
#	include <netdb.h>
#endif
#include <libHX/proc.h>
#include <libHX/socket.h>
#include "internal.h"
#ifdef _WIN32
#	define STUPIDWIN(x) reinterpret_cast(char *, (x))
#else
#	define STUPIDWIN(x) (x)
#endif
#if defined(__sunos__) && !defined(SO_PROTOCOL)
#	define SO_PROTOCOL SO_PROTOTYPE
#endif

static int try_sk_from_env(int fd, const struct addrinfo *ai, const char *intf)
{
	int value = 0;
	socklen_t optlen = sizeof(value);
	int ret = getsockopt(fd, SOL_SOCKET, SO_ACCEPTCONN, STUPIDWIN(&value), &optlen);
	if (ret < 0 || value == 0)
		return -1;
#ifdef _WIN32
	WSAPROTOCOL_INFO protinfo;
	optlen = sizeof(protinfo);
	ret = getsockopt(fd, SOL_SOCKET, SO_PROTOCOL_INFO, STUPIDWIN(&protinfo), &optlen);
	if (ret < 0 || protinfo.iAddressFamily != ai->ai_family ||
	    protinfo.iSocketType != ai->ai_socktype ||
	    protinfo.iProtocol != ai->ai_protocol)
		return -1;
#else
	optlen = sizeof(value);
	ret = getsockopt(fd, SOL_SOCKET, SO_DOMAIN, &value, &optlen);
	if (ret < 0 || value != ai->ai_family)
		return -1;
	optlen = sizeof(value);
	ret = getsockopt(fd, SOL_SOCKET, SO_TYPE, &value, &optlen);
	if (ret < 0 || value != ai->ai_socktype)
		return -1;
	optlen = sizeof(value);
	ret = getsockopt(fd, SOL_SOCKET, SO_PROTOCOL, &value, &optlen);
	if (ret < 0 || value != ai->ai_protocol)
		return -1;
#endif
	struct sockaddr_storage addr;
	memset(&addr, 0, sizeof(addr));
	optlen = sizeof(addr);
	ret = getsockname(fd, (struct sockaddr *)&addr, &optlen);
	if (ret < 0)
		return -1;
	if (sizeof(addr) < optlen)
		optlen = sizeof(addr);
	if (optlen != ai->ai_addrlen || memcmp(&addr, ai->ai_addr, optlen) != 0)
		return -1;
	if (intf == nullptr)
		return fd;
#ifdef SO_BINDTODEVICE
	char ifname[32];
	optlen = sizeof(ifname);
	ret = getsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, ifname, &optlen);
	if (ret < 0)
		return -1;
	else if (optlen < sizeof(ifname))
		ifname[optlen] = '\0';
	else
		ifname[sizeof(ifname)-1] = '\0';
	if (strcmp(intf, ifname) != 0)
		return -1;
#endif
	return fd;
}

EXPORT_SYMBOL int HX_socket_from_env(const struct addrinfo *ai, const char *intf)
{
	int top_fd;
	const char *env_limit = getenv("LISTEN_FDS");
	if (env_limit != nullptr) {
		long x = strtol(env_limit, nullptr, 0);
		if (x > INT_MAX - 3)
			x = INT_MAX - 3;
		top_fd = 3 + x;
	} else {
		env_limit = getenv("HX_LISTEN_TOP_FD");
		long x;
		if (env_limit != nullptr) {
			x = strtol(env_limit, nullptr, 0);
			if (x > INT_MAX)
				x = INT_MAX;
		} else {
			x = HXproc_top_fd();
		}
		top_fd = x;
	}
	for (int fd = 3; fd < top_fd; ++fd)
		if (try_sk_from_env(fd, ai, intf) == fd)
			return fd;
	errno = ENOENT;
	return -1;
}
