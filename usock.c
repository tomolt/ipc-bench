/*
    Measure latency and throughput of IPC using unix pipes


    Copyright (c) 2018 Thomas Oltmann <thomas.oltmann.hhg@gmail.com>
    Copyright (c) 2016 Erik Rigtorp <erik@rigtorp.se>

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

#include "bench.h"

#include <sys/socket.h>

#define SUBJECT "usock"

int main(int argc, char *argv[])
{
	int fds[2];

	bench_mode mode;
	int size;
	char *buf;
	int64_t count, i;
	bench_tm start, stop;

	if (argc != 4) {
		perror("usage: pipe {lt} <message size> <roundtrip count>\n");
		return 1;
	}

	if (argv[1][0] != 'l' && argv[1][0] != 't') {
		perror("argv[1]");
		return 1;
	}

	mode = argv[1][0];
	size = atoi(argv[2]);
	count = atol(argv[3]);

	buf = malloc(size);
	if (buf == NULL) {
		perror("malloc");
		return 1;
	}

	if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == -1) {
		perror("socketpair");
		return 1;
	}

	switch (mode) {

	case BENCH_LATENCY:
		if (!fork()) { /* child */
			for (i = 0; i < count; i++) {

				if (read(fds[1], buf, size) != size) {
					perror("read");
					return 1;
				}

				if (write(fds[1], buf, size) != size) {
					perror("write");
					return 1;
				}
			}
		} else { /* parent */

			bench_time(&start);

			for (i = 0; i < count; i++) {

				if (write(fds[0], buf, size) != size) {
					perror("write");
					return 1;
				}

				if (read(fds[0], buf, size) != size) {
					perror("read");
					return 1;
				}
			}

			bench_time(&stop);

			int64_t latency = bench_latency(start, stop, count);
			printf("\"%s\"\t%li\n", SUBJECT, latency);
		}
		return 0;

	case BENCH_THROUGHPUT:
		if (!fork()) { /* child */
			for (i = 0; i < count; i++) {
				if (read(fds[1], buf, size) != size) {
					perror("read");
					return 1;
				}
			}
		} else { /* parent */

			bench_time(&start);

			for (i = 0; i < count; i++) {
				if (write(fds[0], buf, size) != size) {
					perror("write");
					return 1;
				}
			}

			bench_time(&stop);

			int64_t throughput = bench_throughput(start, stop, size, count);
			printf("\"%s\"\t%li\n", SUBJECT, throughput);
		}
		return 0;
	}
}
