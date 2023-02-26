// SPDX-License-Identifier: MIT
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX/defs.h>
#include <libHX/socket.h>
#ifndef _WIN32
#	include <netdb.h>
#endif
#ifndef AI_V4MAPPED
#	define AI_V4MAPPED 0
#endif

int main(void)
{
	static const char *addrs[] = {
		"::1", "::2", "::ffff:127.0.0.1", "::",
		"127.0.0.1", "127.0.0.2", "1.1.1.1", "255.255.255.255",
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
	return EXIT_SUCCESS;
}
