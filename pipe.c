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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#if defined(_POSIX_TIMERS) && (_POSIX_TIMERS > 0) &&                           \
    defined(_POSIX_MONOTONIC_CLOCK)
#define HAS_CLOCK_GETTIME_MONOTONIC
#endif

#define SUBJECT "pipe"

typedef enum {
	BENCH_LATENCY = 'l',
	BENCH_THROUGHPUT = 't',
} bench_mode;

#ifdef HAS_CLOCK_GETTIME_MONOTONIC
typedef struct timespec bench_tm;
#else
typedef struct timeval bench_tm;
#endif

static inline void bench_time(bench_tm *tm)
{
#ifdef HAS_CLOCK_GETTIME_MONOTONIC
		if (clock_gettime(CLOCK_MONOTONIC, tm) == -1) {
			perror("clock_gettime");
			exit(1);
		}
#else
		if (gettimeofday(tm, NULL) == -1) {
			perror("gettimeofday");
			exit(1);
		}
#endif
}

// TODO this doesn't need to return an int64_t.
static inline int64_t bench_latency(bench_tm start, bench_tm stop, int64_t count)
{
	int64_t dsec = stop.tv_sec - start.tv_sec;
#ifdef HAS_CLOCK_GETTIME_MONOTONIC
	int64_t dnsec = stop.tv_nsec - start.tv_nsec;
#else
	int64_t dnsec = (stop.tv_usec - start.tv_usec) * 1000;
#endif
	int64_t cmb_dnsec = dsec * 1000000000 + dnsec;
	return cmb_dnsec / (count * 2); // times 2 because we measure entire roundtrips
}

static inline int64_t bench_throughput(bench_tm start, bench_tm stop, int size, int64_t count)
{
	int64_t dsec = stop.tv_sec - start.tv_sec;
#ifdef HAS_CLOCK_GETTIME_MONOTONIC
	int64_t dmysec = (stop.tv_nsec - start.tv_nsec) / 1000;
#else
	int64_t dmysec = stop.tv_usec - start.tv_usec;
#endif
	int64_t cmb_dmysec = dsec * 1000000 + dmysec;
	return size * count * 8 / cmb_dmysec; // in Mbit/s
}

int main(int argc, char *argv[])
{
	int ofds[2];
	int ifds[2];

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

	if (pipe(ofds) == -1) {
		perror("pipe");
		return 1;
	}

	if (pipe(ifds) == -1) {
		perror("pipe");
		return 1;
	}

	switch (mode) {

	case BENCH_LATENCY:
		if (!fork()) { /* child */
			for (i = 0; i < count; i++) {

				if (read(ifds[0], buf, size) != size) {
					perror("read");
					return 1;
				}

				if (write(ofds[1], buf, size) != size) {
					perror("write");
					return 1;
				}
			}
		} else { /* parent */

			bench_time(&start);

			for (i = 0; i < count; i++) {

				if (write(ifds[1], buf, size) != size) {
					perror("write");
					return 1;
				}

				if (read(ofds[0], buf, size) != size) {
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
				if (read(ifds[0], buf, size) != size) {
					perror("read");
					return 1;
				}
			}
		} else { /* parent */

			bench_time(&start);

			for (i = 0; i < count; i++) {
				if (write(ifds[1], buf, size) != size) {
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
