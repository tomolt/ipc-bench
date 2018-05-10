/*
    Measure latency and throughput of IPC using unix sockets


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

#define BMB_USE_TEMPLATE 1
typedef int S_party;
#include "bench.h"

static char const *S_ident = "usock";

static void S_mkparties(int size, S_party *p, S_party *c)
{
	int fds[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == -1) {
		perror("socketpair");
		exit(1);
	}
	*p = fds[0];
	*c = fds[1];
}

static inline void S_read(S_party p, char *buf, int size)
{
	if (read(p, buf, size) != size) {
		perror("read");
		exit(1);
	}
}

static inline void S_write(S_party p, char *buf, int size)
{
	if (write(p, buf, size) != size) {
		perror("write");
		exit(1);
	}
}

static void S_teardown(void) {}

