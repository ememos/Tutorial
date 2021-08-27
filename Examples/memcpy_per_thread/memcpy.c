  
/*
 * Micro-test program used to study scalability of Linux kernel code
 * in the case where userspace code is embarrassingly parallel.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <numa.h>
#include <sched.h>
#include "common.h"

#ifdef PREPOPULATE
#define BENCH_NAME "memcpypre"
#else
#define BENCH_NAME "memcpy"
#endif

static uint64_t count = 1 << 29; // default: 4 GB
struct tdata **tdata_main;

static __uint128_t g_lehmer64_state;
static inline uint64_t lehmer64(void) {
  g_lehmer64_state *= 0xda942042e4dd58b5ull;
  return g_lehmer64_state >> 64;
}

static void fill_lehmer64(uint64_t *vec, size_t nelem, uint64_t seed)
{
	size_t i;
	g_lehmer64_state = seed;
	for (i = 0; i < nelem; i++)
		vec[i] = lehmer64();
}

/*
 * Simple thread that just copies in a loop, touching only
 * its own stack page, the source, and the destination, so it doesn't share writable memory
 * with any other thread, and does not call the kernel.
 */
void *do_memcpy_numa(void *argp)
{
	struct tdata *tdata = (struct tdata *)argp;
	uint64_t *destp;
	uint64_t *srcp;
	size_t size = count * sizeof(uint64_t);

    //prepare_memcpy
    struct timespec beg, end;
	double duration;

	/* prepare source */
	srcp = numa_alloc_onnode(count * sizeof(uint64_t), numa_node_of_cpu(tdata->tid%num_proc()));

	if (srcp == NULL) {
		perror("malloc");
		return -1;
	}
	get_time(beg);
	fill_lehmer64(srcp, count, 135432111);
	get_time(end);
	duration = get_duration(beg, end);

	fprintf(stderr, "Initialized random source %'lu bytes in %f sec.\n", count * sizeof(uint64_t), duration);
	fprintf(stderr, "Source generation rate %f GB/sec.\n\n", (count * sizeof(uint64_t) / duration / 1e9));
    //

	cpu_pin(tdata->tid % num_proc());
	destp = numa_alloc_onnode(size, numa_node_of_cpu(tdata->tid % num_proc()));
	if (destp == NULL) {
		perror("malloc");
		return (void *) -1;
	} 
#ifdef PREPOPULATE
	else {
		/* to confirm the actual memory allocation */
		size_t i;
		for (i = 0; i < count; i += (4096 / sizeof(uint64_t)))
			destp[i] = 0;	
	}
#endif

	roi_begin();
	memcpy(destp, srcp, size);
	roi_end();

    numa_free(srcp, size);
	numa_free(destp, size);

	return NULL;
}

void *do_memcpy(void *argp)
{
	struct tdata *tdata = (struct tdata *)argp;
	uint64_t *destp;
	uint64_t *srcp;
	size_t size = count * sizeof(uint64_t);

    //prepare_memcpy
    struct timespec beg, end;
	double duration;

	/* prepare source */
    srcp = malloc(count * sizeof(uint64_t));

	if (srcp == NULL) {
		perror("malloc");
		return -1;
	}

	get_time(beg);
	fill_lehmer64(srcp, count, 135432111);
	get_time(end);
	duration = get_duration(beg, end);

	fprintf(stderr, "Initialized random source %'lu bytes in %f sec.\n", count * sizeof(uint64_t), duration);
	fprintf(stderr, "Source generation rate %f GB/sec.\n\n", (count * sizeof(uint64_t) / duration / 1e9));
	//

	//cpu_pin(tdata->tid % num_proc());
	destp = malloc(size);
	if (destp == NULL) {
		perror("malloc");
		return (void *) -1;
	} 
#ifdef PREPOPULATE
	else {
		/* to confirm the actual memory allocation */
		size_t i;
		for (i = 0; i < count; i += (4096 / sizeof(uint64_t)))
			destp[i] = 0;	
	}
#endif

	roi_begin();
	memcpy(destp, srcp, size);
	roi_end();

    free(srcp);
	free(destp);

	return NULL;
}

int main(int argc, char *argv[])
{
	uint64_t threads;
	double duration;
	
	threads = num_proc();

	if (parse_option(argc, argv, &threads, &count)) {
		fprintf(stderr, "Usage: %s {threads} {count}\n", argv[0]);
		return -1;
	}

#ifdef NUMA
	cpu_pin(0);	
#endif

	init_benchmark(threads);
#ifdef NUMA
	tdata_main = init_tdata_numa();
#else
	tdata_main = init_tdata();
#endif
	if (!tdata_main) return -1;

#ifdef NUMA
	duration = run_threads(tdata_main, do_memcpy_numa);
#else
	duration = run_threads(tdata_main, do_memcpy);
#endif
	fprintf(stderr, "Threads stopped\n\n");
	fprintf(stderr, "Copied source in %lu threads in %f sec.\n", threads,duration);
	fprintf(stderr, "Achieving parallel memory read-write bandwidth: %f GB/sec.\n", (count * sizeof(uint64_t) / duration / 1e9) * threads * 2.0);

#ifdef NUMA
	free_tdata_numa(tdata_main);
#else
	free_tdata(tdata_main);
#endif
	
	print_result(BENCH_NAME, threads, duration, count);

	return 0;
}
