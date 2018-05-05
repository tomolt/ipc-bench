/*
    Measure latency and throughput of IPC using loopback tcp/ip sockets


    Copyright (c) 2018 Thomas Oltmann <thomas.oltmann.hhg@gmail.com>

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use,
    copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following
    conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.
*/

#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

#define BMB_USE_TEMPLATE 1
typedef int S_party;
#include "bench.h"

static char const *S_ident = "tcp";

static void S_mkparties(int size, S_party *p, S_party *c)
{
	int fds[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == -1) {
		perror("socketpair");
		exit(1);
	}
	*p = fds[0];
	*c = fds[1];

	// Setup general socket hints and address info
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; // use IPv4 or IPv6, whichever
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // fill in my IP for me
	struct addrinfo *info;
	int ret = getaddrinfo("127.0.0.1", "3491", &hints, &info);
	if (ret != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
		exit(1);
	}

	// Setup server (child) socket and start listening
	int server = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
	if (server == -1) {
		perror("socket");
		exit(1);
	}
	int yes = 1;
	if (setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("setsockopt");
		exit(1);
	}
	if (bind(server, info->ai_addr, info->ai_addrlen) == -1) {
		perror("bind");
		exit(1);
	}
	if (listen(server, 1) == -1) {
		perror("listen");
		exit(1);
	}

	// Setup client (parent) socket and connect
	int client = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
	if (client == -1) {
		perror("socket");
		exit(1);
	}
	if (connect(client, info->ai_addr, info->ai_addrlen) == -1) {
		perror("connect");
		exit(1);
	}
	
	// Accept client connection
	struct sockaddr_storage their_addr;
	socklen_t addr_size = sizeof(their_addr);
	int client_remote = accept(server, (struct sockaddr *)&their_addr, &addr_size);
	if (client_remote == -1) {
		perror("accept");
		exit(1);
	}
	
	// Fill in parties
	*p = client;
	*c = client_remote;
}

static inline void S_read(S_party p, char *buf, int size)
{
	size_t sofar = 0;
	while (sofar < size) {
		ssize_t len = read(p, buf, size - sofar);
		if (len == -1) {
			perror("read");
			exit(1);
		}
		sofar += len;
	}
}

static inline void S_write(S_party p, char *buf, int size)
{
	if (write(p, buf, size) != size) {
		perror("write");
		exit(1);
	}
}

