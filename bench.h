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

