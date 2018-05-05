/*
    Measure latency and throughput of IPC using unix pipes


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
typedef struct { int r, w; } S_party;
#include "bench.h"

static char const *S_ident = "pipe";

static void S_mkparties(int size, S_party *p, S_party *c)
{
	int p2c[2], c2p[2];
	if (pipe(p2c) == -1) {
		perror("pipe");
		exit(1);
	}
	if (pipe(c2p) == -1) {
		perror("pipe");
		exit(1);
	}
	*p = (S_party){c2p[0], p2c[1]};
	*c = (S_party){p2c[0], c2p[1]};
}

static inline void S_read(S_party p, char *buf, int size)
{
	if (read(p.r, buf, size) != size) {
		perror("read");
		exit(1);
	}
}

static inline void S_write(S_party p, char *buf, int size)
{
	if (write(p.w, buf, size) != size) {
		perror("write");
		exit(1);
	}
}

