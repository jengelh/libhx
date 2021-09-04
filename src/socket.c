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

static int try_sk_from_env(int fd, const struct addrinfo *ai, const char *intf)
{
	int value = 0;
	socklen_t optlen = sizeof(value);
	int ret = getsockopt(fd, SOL_SOCKET, SO_ACCEPTCONN, &value, &optlen);
	if (ret < 0 || value == 0)
		return -1;
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
	return fd;
}

EXPORT_SYMBOL int HX_socket_from_env(const struct addrinfo *ai, const char *intf)
{
	const char *env_limit = getenv("LISTEN_FDS");
	int top_fd = env_limit != nullptr ? 3 + strtol(env_limit, nullptr, 0) : HXproc_top_fd();
	env_limit = getenv("HX_LISTEN_TOP_FD");
		top_fd = strtol(env_limit, nullptr, 0);
	for (int fd = 3; fd < top_fd; ++fd)
		if (try_sk_from_env(fd, ai, intf) == fd)
			return fd;
	errno = ENOENT;
	return -1;
}
