// SPDX-License-Identifier: MIT
#define WIN32_LEAN_AND_MEAN 1
#ifdef __cplusplus
#	include <cstdlib>
#	include <cstdio>
#	include <cstring>
#else
#	include <stdlib.h>
#	include <stdio.h>
#	include <string.h>
#endif
#include <unistd.h>
#ifdef _WIN32
#	include <winsock2.h>
#	include <ws2tcpip.h>
#else
#	include <sys/socket.h>
#	include <netdb.h>
#endif
#include <libHX/init.h>
#include <libHX/io.h>

static int runner(void)
{
	const char id[] = "SSH-2.0-OpenSSH_9.9";
	struct addrinfo *res;
	int fd, ret;

	if ((ret = HX_init()) <= 0) {
		fprintf(stderr, "HX_init: %s\n", strerror(-ret));
		return EXIT_FAILURE;
	}

	fd = socket(AF_INET6, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("socket");
		return EXIT_FAILURE;
	}
	if (getaddrinfo("::1", "22", NULL, &res) < 0) {
		perror("getaddrinfo");
		return EXIT_FAILURE;
	}
	if (connect(fd, res->ai_addr, res->ai_addrlen) < 0) {
		perror("connect");
		return EXIT_FAILURE;
	}
	if (HXio_fullwrite(fd, id, strlen(id)) < 0)
		perror("write");
	close(fd);
	HX_exit();
	return EXIT_SUCCESS;
}

int main(void)
{
	int ret = runner();
	if (ret != EXIT_SUCCESS)
		fprintf(stderr, "FAILED\n");
	return ret;
}
