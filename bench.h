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

#define BENCH_GETTIMEOFDAY 0
#define BENCH_POSIX_MONOTONIC 1

#if defined(_POSIX_TIMERS) && (_POSIX_TIMERS > 0) \
 && defined(_POSIX_MONOTONIC_CLOCK)
#define BENCH_TIMER BENCH_POSIX_MONOTONIC
#endif

#ifndef BENCH_TIMER
#define BENCH_TIMER BENCH_GETTIMEOFDAY
#endif

#if defined(_POSIX_TIMERS) && (_POSIX_TIMERS > 0) &&                           \
    defined(_POSIX_MONOTONIC_CLOCK)
#define HAS_CLOCK_GETTIME_MONOTONIC
#endif

typedef enum {
	BENCH_LATENCY = 'l',
	BENCH_THROUGHPUT = 't',
} bench_mode;

#if BENCH_TIMER == BENCH_POSIX_MONOTONIC
typedef struct timespec bench_tm;
#else
typedef struct timeval bench_tm;
#endif

static inline void bench_time(bench_tm *tm)
{
#if BENCH_TIMER == BENCH_POSIX_MONOTONIC
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
#if BENCH_TIMER == BENCH_POSIX_MONOTONIC
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
#if BENCH_TIMER == BENCH_POSIX_MONOTONIC
	int64_t dmysec = (stop.tv_nsec - start.tv_nsec) / 1000;
#else
	int64_t dmysec = stop.tv_usec - start.tv_usec;
#endif
	int64_t cmb_dmysec = dsec * 1000000 + dmysec;
	return size * count * 8 / cmb_dmysec; // in Mbit/s
}

