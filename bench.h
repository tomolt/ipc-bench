/*
	Common code shared by all parts of the ipc-bench
	

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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define BMB_GETTIMEOFDAY 0
#define BMB_POSIX_MONOTONIC 1

#if defined(_POSIX_TIMERS) && (_POSIX_TIMERS > 0) \
 && defined(_POSIX_MONOTONIC_CLOCK)
#define BMB_TIMER BMB_POSIX_MONOTONIC
#endif

#ifndef BMB_TIMER
#define BMB_TIMER BMB_GETTIMEOFDAY
#endif

typedef enum {
	BMB_LATENCY = 0x1,
	BMB_THROUGHPUT = 0x2,
} bmb_mode;

#if BMB_TIMER == BMB_POSIX_MONOTONIC
typedef struct timespec bmb_tm;
#else
typedef struct timeval bmb_tm;
#endif

static inline void bmb_loadcfg(int argc, char *argv[],
	bmb_mode *mode, int *size, int64_t *count)
{
#if 0
	// mode
	switch (argc) {
	case 1:
		*mode = BMB_LATENCY | BMB_THROUGHPUT;
		break;
	case 2:
		for (char *c = argv[1]; *c != '\0'; ++c) {
			switch (*c) {
			case 'l': *mode |= BMB_LATENCY;    break;
			case 't': *mode |= BMB_THROUGHPUT; break;
			default:
				fprintf(stderr, "Invalid argument '%c'.\n", *c);
				exit(1);
			}
		}
		break;
	default:
		fprintf(stderr, "Invalid number of arguments.\n");
		exit(1);
	}
#endif
	// mode
	switch (argc) {
	case 2:
		switch (argv[1][0]) {
		case 'l': *mode = BMB_LATENCY;    break;
		case 't': *mode = BMB_THROUGHPUT; break;
		default:
			fprintf(stderr, "Invalid argument '%c'.\n", argv[1][0]);
			exit(1);
		}
		break;
	default:
		fprintf(stderr, "Invalid number of arguments.\n");
		exit(1);
	}
	// size
	char const *sizeStr = getenv("BMB_SIZE"); // TODO better name for 'size'
	if (sizeStr == NULL) {
		fprintf(stderr, "Environment variables not set correctly.\n");
		exit(1);
	}
	*size = atoi(sizeStr); // TODO syntax error handling
	// count
	char const *countStr = getenv("BMB_COUNT");
	if (countStr == NULL) {
		fprintf(stderr, "Environment variables not set correctly.\n");
		exit(1);
	}
	*count = atol(countStr);
}

static inline void bmb_time(bmb_tm *tm)
{
#if BMB_TIMER == BMB_POSIX_MONOTONIC
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
static inline int64_t bmb_latency(bmb_tm start, bmb_tm stop, int64_t count)
{
	int64_t dsec = stop.tv_sec - start.tv_sec;
#if BMB_TIMER == BMB_POSIX_MONOTONIC
	int64_t dnsec = stop.tv_nsec - start.tv_nsec;
#else
	int64_t dnsec = (stop.tv_usec - start.tv_usec) * 1000;
#endif
	int64_t cmb_dnsec = dsec * 1000000000 + dnsec;
	return cmb_dnsec / (count * 2); // times 2 because we measure entire roundtrips
}

static inline int64_t bmb_throughput(bmb_tm start, bmb_tm stop, int size, int64_t count)
{
	int64_t dsec = stop.tv_sec - start.tv_sec;
#if BMB_TIMER == BMB_POSIX_MONOTONIC
	int64_t dmysec = (stop.tv_nsec - start.tv_nsec) / 1000;
#else
	int64_t dmysec = stop.tv_usec - start.tv_usec;
#endif
	int64_t cmb_dmysec = dsec * 1000000 + dmysec;
	return size * count * 8 / cmb_dmysec; // in Mbit/s
}

#if BMB_USE_TEMPLATE == 1
static char const *S_ident;
static void S_mkparties(int size, S_party *p, S_party *c);
static void S_teardown(void);
static void S_read(S_party party, char *buf, int size);
static void S_write(S_party party, char *buf, int size);

int main(int argc, char *argv[])
{
	bmb_mode mode;
	int size;
	int64_t count;
	bmb_tm start, stop;
	S_party p, c;
	bmb_loadcfg(argc, argv, &mode, &size, &count);
	S_mkparties(size, &p, &c);
	char *buf = malloc(size);
	if (buf == NULL) {
		perror("malloc");
		exit(1);
	}
	switch (mode) {
	case BMB_LATENCY:
		if (!fork()) { /* child */
			for (int64_t i = 0; i < count; i++) {
				S_read(c, buf, size);
				S_write(c, buf, size);
			}
		} else { /* parent */
			bmb_time(&start);
			for (int64_t i = 0; i < count; i++) {
				S_write(p, buf, size);
				S_read(p, buf, size);
			}
			bmb_time(&stop);
			int64_t latency = bmb_latency(start, stop, count);
			printf("\"%s\"\t%li\n", S_ident, latency);
			S_teardown(); // TODO probably move teardown into child and let parent wait for exit
		}
		return 0;
	case BMB_THROUGHPUT:
		if (!fork()) { /* child */
			for (int64_t i = 0; i < count; i++) {
				S_read(c, buf, size);
			}
		} else { /* parent */
			bmb_time(&start);
			for (int64_t i = 0; i < count; i++) {
				S_write(p, buf, size);
			}
			bmb_time(&stop);
			int64_t throughput = bmb_throughput(start, stop, size, count);
			printf("\"%s\"\t%li\n", S_ident, throughput);
			S_teardown(); // TODO probably move teardown into child and let parent wait for exit
		}
		return 0;
	}
}
#endif

