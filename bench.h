
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

