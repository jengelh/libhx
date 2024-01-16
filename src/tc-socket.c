// SPDX-License-Identifier: MIT
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX/defs.h>
#include <libHX/socket.h>
#ifndef _WIN32
#	include <netdb.h>
#	include <unistd.h>
#endif
#ifndef AI_V4MAPPED
#	define AI_V4MAPPED 0
#endif
#include "internal.h"

static int t_parse(void)
{
	char host[32] = "bogus";
	uint16_t port = 4321;
	if (HX_addrport_split("[::1]", host, sizeof(host), nullptr) != 1 ||
	    strcmp(host, "::1") != 0)
		return 1;
	if (HX_addrport_split("[]", host, sizeof(host), nullptr) != 1 ||
	    strcmp(host, "") != 0)
		return 1;
	if (HX_addrport_split("", host, sizeof(host), nullptr) != 1 ||
	    strcmp(host, "") != 0)
		return 1;
	if (HX_addrport_split("[]:", host, sizeof(host), &port) != 1 ||
	    strcmp(host, "") != 0 || port != 0)
		return 1;
	return 0;
}

static int t_local(void)
{
	static const char *addrs[] = {
		"::1", "::2", "::ffff:127.0.0.1", "::",
		"[::1]", "[::2]", "[::ffff:127.0.0.1]", "[::]",
		"127.0.0.1", "127.0.0.2", "1.1.1.1", "255.255.255.255",
		"[127.0.0.1]", "[127.0.0.2]", "[1.1.1.1]", "[255.255.255.255]",
	};
	for (size_t i = 0; i < ARRAY_SIZE(addrs); ++i) {
		char host[32] = {};
		uint16_t port = 0;

		int ret = HX_addrport_split(addrs[i], host, sizeof(host), &port);
		if (ret >= 0)
			printf("Parse \"%s\" -> [%s]:%hu\n", addrs[i], host, port);
		else
			return EXIT_FAILURE;

		printf("%-16s\t", addrs[i]);
		int lcl = HX_ipaddr_is_local(addrs[i], AI_V4MAPPED);
		if (lcl < 0) {
			printf("%s\n", strerror(-lcl));
			return EXIT_FAILURE;
		}
		printf("%d\n", lcl);
	}

	char host[32] = {};
	uint16_t port = 0;
	int ret = HX_addrport_split("[fe80::1]:80", host, sizeof(host), &port);
	if (ret < 0 || port != 80)
		return EXIT_FAILURE;
	port = 443;
	ret = HX_addrport_split("::80", host, sizeof(host), &port);
	if (ret < 0 || port != 443)
		return EXIT_FAILURE;
	ret = HX_addrport_split(":::80", host, sizeof(host), &port);
	if (ret < 0 || port != 443)
		return EXIT_FAILURE;
	ret = HX_addrport_split("0.0.0.0", host, sizeof(host), &port);
	if (ret < 0 || port != 443)
		return EXIT_FAILURE;
	ret = HX_addrport_split("0.0.0.0:80", host, sizeof(host), &port);
	if (ret < 0 || port != 80)
		return EXIT_FAILURE;

	int fd = HX_inet_connect("::1", 80, 0);
	if (fd >= 0) {
		printf("Connected to [::1]:80\n");
		close(fd);
	} else {
		fprintf(stderr, "HX_inet_connect [::1]:80: %s\n", strerror(-fd));
	}
	fd = HX_inet_connect("::", 80, 0);
	if (fd >= 0) {
		printf("Connected to [::]:80\n");
		close(fd);
	} else {
		fprintf(stderr, "HX_inet_connect [::]:80: %s\n", strerror(-fd));
	}
	return EXIT_SUCCESS;
}

int main(void)
{
	int ret = t_parse();
	if (ret != 0)
		return ret;
	ret = t_local();
	if (ret != EXIT_SUCCESS)
		return ret;
	return EXIT_SUCCESS;
}
