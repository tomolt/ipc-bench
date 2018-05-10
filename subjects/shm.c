/*
    Measure latency and throughput of IPC using unix shm and semaphores


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

#include <string.h>
#include <sys/shm.h>
// #include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>

typedef struct { sem_t s; char b[]; } unidir;

#define BMB_USE_TEMPLATE 1
typedef struct { unidir *r, *w; } S_party;
#include "bench.h"

static char const *S_ident = "shm";

static char const *SHMID = "ipc_bench_shm";

static int shm_fd;

static void S_mkparties(int size, S_party *p, S_party *c)
{
	shm_fd = shm_open(SHMID, O_CREAT | O_RDWR, 0666);
	if (shm_fd == -1) {
		perror("shm_open");
		exit(1);
	}
	int const udirsz = sizeof(unidir) + size;
	if (ftruncate(shm_fd, udirsz * 2) == -1) {
		perror("ftruncate");
		exit(1);
	}
	char *mapped = mmap(NULL, udirsz * 2, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if (mapped == MAP_FAILED) {
		perror("mmap");
		exit(1);
	}
	unidir *a = (unidir *)mapped, *b = (unidir *)(mapped + udirsz);
	sem_init(&a->s, 1, 0);
	sem_init(&b->s, 1, 0);
	p->r = c->w = a;
	p->w = c->r = b;
}

static void S_teardown(void)
{
	if (shm_unlink(SHMID) == -1) {
		perror("shm_unlink");
		exit(1);
	}
}

static inline void S_read(S_party p, char *buf, int size)
{
	sem_wait(&p.r->s);
}

static inline void S_write(S_party p, char *buf, int size)
{
	memcpy(p.w->b, buf, size);
	sem_post(&p.w->s);
}

