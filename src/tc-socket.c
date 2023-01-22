#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX/defs.h>
#include <libHX/socket.h>
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
		printf("%-16s\t", addrs[i]);
		int lcl = HX_ipaddr_is_local(addrs[i], AI_V4MAPPED);
		if (lcl < 0) {
			printf("%s\n", strerror(-lcl));
			return EXIT_FAILURE;
		}
		printf("%d\n", lcl);
	}
	return EXIT_SUCCESS;
}
