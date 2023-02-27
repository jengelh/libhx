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
#	include <fcntl.h>
#	include <netdb.h>
#	include <unistd.h>
#	include <netinet/in.h>
#	include <sys/socket.h>
#	include <sys/stat.h>
#endif
#ifdef HAVE_SYS_UN_H
#	include <sys/un.h>
#endif
#ifdef __linux__
#	include <linux/rtnetlink.h>
#endif
#ifdef __OpenBSD__
#	include <net/route.h>
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
#ifndef AI_V4MAPPED
#	define AI_V4MAPPED 0
#endif

/**
 * Return the pointer to the singular colon character, any other input
 * yields nullptr.
 */
static inline const char *has_exactly_one_colon(const char *s)
{
	s = strchr(s, ':');
	if (s == nullptr)
		return nullptr;
	return strchr(s + 1, ':') == nullptr ? s : nullptr;
}

/**
 * @spec:	"[" HOST-ANY "]" [ ":" PORT ]
 * 		HOST-NAME [ ":" PORT ]
 * 		HOST-IPV4 [ ":" PORT ]
 * 		HOST-IPV6
 * @host:	buffer for placing the extracted hostname
 * 		(can overlap @spec)
 * @hsize:	buffer size for @host
 * @port:	storage space for extracted port number
 * 		(can be nullptr)
 *
 * Returns <0 (error code) if unparsable or if the output buffer is too small.
 * Success if on >=0.
 */
int HX_addrport_split(const char *spec, char *host,
    size_t hbufsz, uint16_t *pport)
{
	if (*spec == '[') {
		/* We also happen to allow IPv4 addrs and hostnames in [] */
		++spec;
		const char *end = strchr(spec, ']');
		if (end == nullptr)
			return -EINVAL;
		unsigned long hlen = end - spec;
		if (hlen >= hbufsz)
			return -E2BIG;
		if (*++end == '\0')
			return 1;
		if (*end++ != ':')
			return -EINVAL;
		char *nend = nullptr;
		uint16_t port = strtoul(end, &nend, 10);
		if (nend == nullptr || *nend != '\0')
			return -EINVAL;
		memmove(host, spec, hlen);
		host[hlen] = '\0';
		if (pport == nullptr)
			return 2;
		*pport = port;
		return 2;
	}
	const char *onecolon = has_exactly_one_colon(spec);
	if (onecolon != nullptr) {
		unsigned long hlen = onecolon - spec;
		if (hlen >= hbufsz)
			return -E2BIG;
		char *nend = nullptr;
		uint16_t port = strtoul(onecolon + 1, &nend, 10);
		if (nend == nullptr || *nend != '\0')
			return -EINVAL;
		memmove(host, spec, hlen);
		host[hlen] = '\0';
		if (pport == nullptr)
			return 2;
		*pport = port;
		return 2;
	}
	size_t hlen = strlen(spec);
	if (hlen >= SIZE_MAX || ++hlen >= hbufsz)
		return -E2BIG;
	memmove(host, spec, hlen);
	return 1;
}

static int HX_inet_lookup(const char *host, uint16_t port, unsigned int xflags,
    struct addrinfo **res)
{
	struct addrinfo hints = {};
#if defined(AI_V4MAPPED)
	hints.ai_flags    = AI_V4MAPPED | xflags;
#else
	hints.ai_flags    = xflags;
#endif
	hints.ai_family   = AF_INET6;
	hints.ai_socktype = SOCK_STREAM;

	char portbuf[HXSIZEOF_Z32];
	snprintf(portbuf, sizeof(portbuf), "%hu", port);
	return getaddrinfo(host, port == 0 ? nullptr : portbuf, &hints, res);
}

int HX_inet_connect(const char *host, uint16_t port, unsigned int oflags)
{
	struct addrinfo *aires = nullptr;
	int ret = HX_inet_lookup(host, port, AI_ADDRCONFIG, &aires);
	int saved_errno = 0;
	for (const struct addrinfo *r = aires; r != nullptr; r = r->ai_next) {
		int fd = socket(r->ai_family, r->ai_socktype, r->ai_protocol);
		if (fd < 0) {
			if (saved_errno == 0)
				saved_errno = errno;
			continue;
		}
#ifdef O_NONBLOCK
		if (oflags & O_NONBLOCK) {
			int flags = fcntl(fd, F_GETFL, 0);
			if (flags < 0)
				flags = 0;
			flags |= O_NONBLOCK;
			if (fcntl(fd, F_SETFL, flags) != 0) {
				saved_errno = errno;
				close(fd);
				continue;
			}
		}
#endif
		ret = connect(fd, r->ai_addr, r->ai_addrlen);
		if (ret == 0) {
			freeaddrinfo(aires);
			return fd;
		}
#ifdef O_NONBLOCK
		if ((errno == EWOULDBLOCK || errno == EINPROGRESS) &&
		    (oflags & O_NONBLOCK)) {
			freeaddrinfo(aires);
			return fd;
		}
#endif
		saved_errno = errno;
		close(fd);
	}
	if (aires == nullptr && saved_errno == 0)
		saved_errno = EHOSTUNREACH;
	if (aires != nullptr)
		freeaddrinfo(aires);
	return -(errno = saved_errno);
}

static int HX_gai_listen(const struct addrinfo *r)
{
	int fd = socket(r->ai_family, r->ai_socktype, r->ai_protocol);
	if (fd < 0)
		return -2;
	static const int y = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, STUPIDWIN(&y), sizeof(y)) < 0)
		/* warn setsockopt: %s, strerror(errno)) */ ;
	int ret = bind(fd, r->ai_addr, r->ai_addrlen);
	if (ret != 0) {
		int se = errno;
		close(fd);
		errno = se;
		return -1;
	}
	ret = listen(fd, SOMAXCONN);
	if (ret != 0) {
		int se = errno;
		close(fd);
		errno = se;
		return -2;
	}
	return fd;
}

int HX_inet_listen(const char *host, uint16_t port)
{
	struct addrinfo *aires = nullptr;
	int ret = HX_inet_lookup(host, port, AI_PASSIVE, &aires);
	if (ret != 0)
		;
	int saved_errno = EHOSTUNREACH;
	bool use_env = getenv("HX_LISTEN_TOP_FD") != nullptr || getenv("LISTEN_FDS") != nullptr;
	for (const struct addrinfo *r = aires; r != nullptr; r = r->ai_next) {
		if (use_env) {
			int fd = HX_socket_from_env(r, nullptr);
			if (fd >= 0) {
				freeaddrinfo(aires);
				return fd;
			}
		}
		int fd = HX_gai_listen(r);
		if (fd >= 0) {
			freeaddrinfo(aires);
			return fd;
		}
		saved_errno = errno;
		if (fd == -2)
			continue;
		break;
	}
	if (aires != nullptr)
		freeaddrinfo(aires);
	return -(errno = saved_errno);
}

int HX_local_listen(const char *path)
{
#ifdef HAVE_SYS_UN_H
	struct sockaddr_un u;
	if (strlen(path) >= sizeof(u.sun_path))
		return -EINVAL;
	u.sun_family = AF_LOCAL;
	strcpy(u.sun_path, path);
	struct addrinfo r = {};
	r.ai_flags = AI_PASSIVE;
	r.ai_family = AF_LOCAL;
	r.ai_socktype = SOCK_STREAM;
	r.ai_addrlen = sizeof(u) - sizeof(u.sun_path) + strlen(u.sun_path) + 1;
	r.ai_addr = reinterpret_cast(struct sockaddr *, &u);
	bool use_env = getenv("HX_LISTEN_TOP_FD") != nullptr || getenv("LISTEN_FDS") != nullptr;
	if (use_env) {
		int fd = HX_socket_from_env(&r, nullptr);
		if (fd >= 0)
			return fd;
	}
	int ret = HX_gai_listen(&r);
	if (ret >= 0)
		return ret; /* fd */
	if (ret == -2 || errno != EADDRINUSE)
		return -errno;

	struct stat sb;
	ret = stat(path, &sb);
	if (ret < 0)
		return -errno;
	if (!S_ISSOCK(sb.st_mode))
		return -ENOTSOCK;

	int testfd = socket(AF_LOCAL, SOCK_STREAM, 0);
	if (testfd < 0)
		return -errno;
	ret = connect(testfd, r.ai_addr, r.ai_addrlen);
	close(testfd);
	if (ret == 0)
		return -EADDRINUSE;

	/* There will be a TOCTOU report, but what can you do... */
	ret = unlink(path);
	if (ret < 0 && errno != ENOENT)
		return -errno;
	ret = HX_gai_listen(&r);
	if (ret >= 0)
		return ret; /* fd */
	return -errno;
#else
	return -EPROTONOSUPPORT;
#endif
}

static int try_sk_from_env(int fd, const struct addrinfo *ai, const char *intf)
{
	int value = 0;
	socklen_t optlen = sizeof(value);
	int ret = getsockopt(fd, SOL_SOCKET, SO_ACCEPTCONN, STUPIDWIN(&value), &optlen);
	if (ret < 0 && errno != ENOPROTOOPT)
		/*
		 * E.g. OpenBSD's getsockopt does not recognize this - even
		 * though the flag with the same name exists and is known.
		 */
		return -1;
	if (ret == 0 && value == 0)
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

#ifdef __linux__
static int linux_sockaddr_local3(int sk, const void *buf, size_t bufsize)
{
	if (send(sk, buf, bufsize, 0) < 0)
		return -errno;
	char rsp[4096];
	ssize_t ret = recv(sk, rsp, sizeof(rsp), 0);
	if (ret < 0)
		return -errno;
	else if (static_cast(size_t, ret) < sizeof(struct nlmsghdr))
		return -EIO;
	struct nlmsghdr nlh;
	memcpy(&nlh, rsp, sizeof(nlh));
	if (!NLMSG_OK(&nlh, ret))
		return -EIO;
	const struct rtmsg *rtm = static_cast(void *, rsp + NLMSG_HDRLEN);
	return rtm->rtm_type == RTN_LOCAL;
}

static int linux_sockaddr_local2(const struct sockaddr *sa, socklen_t sl)
{
	int sk = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE);
	if (sk < 0)
		return -errno;
	struct {
		struct nlmsghdr nh;
		struct rtmsg rth;
		char attrbuf[4096];
	} req;
	memset(&req, 0, sizeof(req));
	req.nh.nlmsg_len     = NLMSG_LENGTH(sizeof(req.rth));
	req.nh.nlmsg_flags   = NLM_F_REQUEST;
	req.nh.nlmsg_type    = RTM_GETROUTE;
	req.rth.rtm_family   = sa->sa_family;
	req.rth.rtm_protocol = RTPROT_UNSPEC;
	req.rth.rtm_type     = RTN_UNSPEC;
	req.rth.rtm_scope    = RT_SCOPE_UNIVERSE;
	req.rth.rtm_table    = RT_TABLE_UNSPEC;
	struct rtattr *rta   = reinterpret_cast(struct rtattr *,
	                       reinterpret_cast(char *, &req) + NLMSG_ALIGN(req.nh.nlmsg_len));
	rta->rta_type        = RTA_DST;

	int ret = -ENODATA;
	if (sa->sa_family == AF_INET6) {
		const struct in6_addr *ad = &reinterpret_cast(const struct sockaddr_in6 *, sa)->sin6_addr;
		req.rth.rtm_dst_len = 16;
		rta->rta_len = RTA_LENGTH(16);
		req.nh.nlmsg_len = NLMSG_ALIGN(req.nh.nlmsg_len) + rta->rta_len;
		memcpy(RTA_DATA(rta), ad, 16);
	} else if (sa->sa_family == AF_INET) {
		struct in_addr ad = reinterpret_cast(const struct sockaddr_in *, sa)->sin_addr;
		req.rth.rtm_dst_len = 4;
		rta->rta_len = RTA_LENGTH(4);
		req.nh.nlmsg_len = NLMSG_ALIGN(req.nh.nlmsg_len) + rta->rta_len;
		memcpy(RTA_DATA(rta), &ad, 4);
	}
	ret = linux_sockaddr_local3(sk, &req, req.nh.nlmsg_len);
	close(sk);
	return ret;
}
#endif

#ifdef __OpenBSD__
static int openbsd_sockaddr_local3(int rsk, const void *buf, size_t bufsize)
{
	ssize_t ret = send(rsk, buf, bufsize, 0);
	if (ret < 0)
		return -errno;
	else if (ret != bufsize)
		return -EIO;
	struct rt_msghdr rsp;
	do {
		ret = recv(rsk, &rsp, sizeof(rsp), 0);
	} while (ret > 0 && (rsp.rtm_version != RTM_VERSION ||
	         rsp.rtm_seq != 1 || rsp.rtm_pid != getpid()));
	return rsp.rtm_flags & RTF_LOCAL;
}

static int openbsd_sockaddr_local2(const struct sockaddr *sa, socklen_t sl)
{
	int sk = socket(AF_ROUTE, SOCK_RAW, AF_UNSPEC);
	if (sk < 0)
		return -errno;
	struct {
		struct rt_msghdr rtm;
		char ab[512];
	} req;
	memset(&req, 0, sizeof(req));
	req.rtm.rtm_type = RTM_GET;
	req.rtm.rtm_version = RTM_VERSION;
	req.rtm.rtm_flags = RTF_STATIC | RTF_UP | RTF_HOST | RTF_GATEWAY;
	req.rtm.rtm_seq = 1;
	req.rtm.rtm_addrs = /*RTA_IFP |*/ RTA_DST;
	req.rtm.rtm_tableid = getrtable();
	req.rtm.rtm_hdrlen = sizeof(req.rtm);
	memcpy(req.ab, sa, sl);
	req.rtm.rtm_msglen = sizeof(req.rtm) + sl;
	int ret = openbsd_sockaddr_local3(sk, &req, req.rtm.rtm_msglen);
	close(sk);
	return ret;
}
#endif

EXPORT_SYMBOL int HX_sockaddr_is_local(const struct sockaddr *sa, socklen_t sl,
    unsigned int flags)
{
	struct sockaddr_in xl = {};

	if (sa->sa_family == AF_INET6) {
		if (sl < sizeof(struct sockaddr_in6))
			return -EINVAL;
	} else if (sa->sa_family == AF_INET) {
		if (sl < sizeof(struct sockaddr_in))
			return -EINVAL;
	}
#ifdef HAVE_SYS_UN_H
	else if (sa->sa_family == AF_UNIX) {
		if (sl < sizeof(struct sockaddr_un))
			return 1;
	}
#endif
	else {
		return -EPROTONOSUPPORT;
	}
	if (flags & AI_V4MAPPED && sa->sa_family == AF_INET6) {
		/*
		 * Preprocess mapped addresses, becuase kernel interfaces do
		 * not support them.
		 */
		const struct in6_addr *ad = &reinterpret_cast(const struct sockaddr_in6 *, sa)->sin6_addr;
		static const uint8_t mappedv4[] = {0,0,0,0, 0,0,0,0, 0,0,0xff,0xff};
		if (memcmp(ad, mappedv4, 12) == 0) {
			xl.sin_family = AF_INET;
			memcpy(&xl.sin_addr, &ad->s6_addr[12], 4);
			sa = reinterpret_cast(struct sockaddr *, &xl);
			sl = sizeof(xl);
		}
	}
#if defined(__linux__)
	return linux_sockaddr_local2(sa, sl);
#elif defined(__OpenBSD__)
	return openbsd_sockaddr_local2(sa, sl);
#else
	if (sa->sa_family == AF_INET) {
		struct in_addr a = reinterpret_cast(const struct sockaddr_in *, sa)->sin_addr;
		return ntohl(a.s_addr) >> 24 == 127;
	} else if (sa->sa_family == AF_INET6) {
		return IN6_IS_ADDR_LOOPBACK(&reinterpret_cast(const struct sockaddr_in6 *, sa)->sin6_addr);
	}
#endif
	return -EPROTONOSUPPORT;
}

EXPORT_SYMBOL int HX_ipaddr_is_local(const char *addr, unsigned int flags)
{
	struct addrinfo hints = {.ai_flags = flags & AI_V4MAPPED};
	struct addrinfo *r = nullptr;
	int err = getaddrinfo(addr, nullptr, &hints, &r);
	if (err != 0) {
		freeaddrinfo(r);
		return 0;
	}
	int lcl = HX_sockaddr_is_local(r->ai_addr, r->ai_addrlen, hints.ai_flags);
	freeaddrinfo(r);
	return lcl;
}
