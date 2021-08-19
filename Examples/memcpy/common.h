#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <time.h>
#include <locale.h>
#include <sys/sysinfo.h>

#define get_time(val) clock_gettime(CLOCK_MONOTONIC, &(val))
#define get_duration(beg, end) (((end).tv_sec - (beg).tv_sec) + ((end).tv_nsec - (beg).tv_nsec) / 1e9)


struct benchmark {
	int init;
	int threads;
	char *cmd_roi_before;
	char *cmd_roi_after;
	pthread_barrier_t barriers[4]; /* two for begin, two for end */
	struct timespec beg, end;
} benchmark = {0, 0, NULL, NULL, };

struct tdata {
	int tid;
	void *data;
};

static inline int num_proc() {
	return sysconf(_SC_NPROCESSORS_ONLN);
}

/* usage: [executable] {#threads} {#count option}
 * Save the first argument to *threads_ptr and the third argument to *count_ptr.
 * if {#count option} is not needed, call with count_ptr = NULL.
 * if each argument is not provided by user, this func does not touch its pointer.
 * Thus, before calling this func, each value should be set as its default value.
 * Return 0 if no errors, 1 if error occurs..
 */
static inline int parse_option(int argc, char **argv, uint64_t *threads_ptr, uint64_t *count_ptr) {
	uint64_t threads, count;
	char *endp;
	setlocale(LC_NUMERIC, "");
	
	switch (argc) {
	case 3:
			if (!count_ptr) {
				fprintf(stderr, "Invalid number of arguments\n");
				return -1;
			}
			count = strtoul(argv[2], &endp, 10);
			if (*endp != '\0') {
				fprintf(stderr, "Invalid count argument %s\n", argv[2]); 
				return -1;
			}
			*count_ptr = count;
			/* fall through */
	case 2:
			threads = strtoul(argv[1], &endp, 10);
			if (*endp != '\0' || threads >= (1UL << 32)) {
				fprintf(stderr, "Invalid threads argument %s\n", argv[1]); 
				return -1;
			}
			if (threads != 0) /* if threads == 0, use the default value. */
				*threads_ptr = threads;
			/* fall through */
	case 1:
			return 0;
	default:
			fprintf(stderr, "Invalid number of arguments\n");
			return -1;
	}

	return 0;
}

static inline int init_benchmark(int threads) {
	int i;
	int ret;

	benchmark.threads = threads;
	for (i = 0; i < 4; i++) {
		ret = pthread_barrier_init(&benchmark.barriers[i], NULL, threads + 1);
		if (ret) {
			fprintf(stderr, "failed to init barriers\n");
			return ret;
		}
	}
	benchmark.cmd_roi_before = getenv("ROI_BEFORE");
	benchmark.cmd_roi_after = getenv("ROI_AFTER");
	benchmark.init = 1;
	
	return 0;
}

static inline struct tdata **init_tdata_numa() {
	struct tdata **ret;
	int i;
	int node;
	int core_id;

	if (!benchmark.init) {
		fprintf(stderr, "ERROR: you should call init_benchmark() first.\n");
		exit(-1);
	}

	node = numa_node_of_cpu(0); /* on main thread */
	ret = (struct tdata **) numa_alloc_onnode(benchmark.threads * sizeof(struct tdata *), node);
	if (!ret) {
		fprintf(stderr, "ERROR: failed to allocate tdata\n");
		return NULL;
	}

	for (i = 0; i < benchmark.threads; ++i) {
		core_id = i % num_proc();
		node = numa_node_of_cpu(core_id);
		ret[i] = (struct tdata *) numa_alloc_onnode(sizeof(struct tdata), node);
		if (!ret[i]) goto error;
		ret[i]->tid = i;
		ret[i]->data = NULL;
	}

	return ret;	

error:
	for (i = 0; i < benchmark.threads; ++i) {
		if (ret[i])
			numa_free(ret[i], sizeof(struct tdata));
	}
	numa_free(ret, benchmark.threads * sizeof(struct tdata *));
	return NULL;
}

static inline struct tdata **init_tdata() {
	struct tdata **ret;
	int i;

	if (!benchmark.init) {
		fprintf(stderr, "ERROR: you should call init_benchmark() first.\n");
		exit(-1);
	}

	ret = (struct tdata **) calloc(benchmark.threads, sizeof(struct tdata *));
	if (!ret) {
		fprintf(stderr, "ERROR: failed to allocate tdata\n");
		return NULL;
	}

	for (i = 0; i < benchmark.threads; ++i) {
		ret[i] = (struct tdata *) malloc(sizeof(struct tdata));
		if (!ret[i]) goto error;
		ret[i]->tid = i;
		ret[i]->data = NULL;
	}

	return ret;	

error:
	for (i = 0; i < benchmark.threads; ++i) {
		if (ret[i])
			free(ret[i]);
	}
	free(ret);
	return NULL;
}

static inline void free_tdata_numa(struct tdata **tdata) {
	int i;
	
	if (!benchmark.init) {
		fprintf(stderr, "ERROR: you should call init_benchmark() first.\n");
		exit(-1);
	}

	for (i = 0; i < benchmark.threads; ++i)
		numa_free(tdata[i], sizeof(struct tdata));
	numa_free(tdata, benchmark.threads * sizeof(struct tdata *));
}

static inline void free_tdata(struct tdata **tdata) {
	int i;
	
	if (!benchmark.init) {
		fprintf(stderr, "ERROR: you should call init_benchmark() first.\n");
		exit(-1);
	}

	for (i = 0; i < benchmark.threads; ++i)
		free(tdata[i]);
	free(tdata);
}

static inline void roi_begin_main() {
	if (!benchmark.init) {
		fprintf(stderr, "ERROR: you should call init_benchmark() first.\n");
		exit(-1);
	}
	pthread_barrier_wait(&benchmark.barriers[0]);
	sleep(1); /* wait for sub-threads stuck at the second barrier. */
	fprintf(stderr, "[INFO] ROI begins\n");
	if (benchmark.cmd_roi_before) {
		if (system(benchmark.cmd_roi_before))
			fprintf(stderr, "[INFO] ROI before command exit with non-zero status\n");
	}
	get_time(benchmark.beg);
	pthread_barrier_wait(&benchmark.barriers[1]);
}

static inline void roi_end_main() {
	pthread_barrier_wait(&benchmark.barriers[2]);
	get_time(benchmark.end);
	if (benchmark.cmd_roi_after) {
		if (system(benchmark.cmd_roi_after))
			fprintf(stderr, "[INFO] ROI after command exit with non-zero status\n");
	}
	fprintf(stderr, "[INFO] ROI ends\n");
	pthread_barrier_wait(&benchmark.barriers[3]);
}

static inline void roi_begin() {
	pthread_barrier_wait(&benchmark.barriers[0]);
	pthread_barrier_wait(&benchmark.barriers[1]);
}

static inline void roi_end() {
	pthread_barrier_wait(&benchmark.barriers[2]);
	sleep(1); /* do not disturb get_time of main thread */
	pthread_barrier_wait(&benchmark.barriers[3]);
}

static inline int cpu_pin(int cpu) {
	cpu_set_t mask;
	int ret;
	CPU_ZERO(&mask);
	CPU_SET(cpu, &mask);
	ret = sched_setaffinity(0, sizeof(cpu_set_t), &mask);
	if (ret)
		fprintf(stderr, "ERROR: sched_setaffinity failed\n");
	return ret;
}

static inline double run_threads(struct tdata **tdata, void *(*func) (void *argp)) {
	uint64_t threads = benchmark.threads;	
	pthread_t thread_id[threads];
	int i;
	int err;
	
	for (i = 0; i < threads; i++) {
		err = pthread_create(&thread_id[i], NULL, func, (void *) tdata[i]);
		if (err) {
			perror("pthread_create");
			exit(1);
		}
	}

	roi_begin_main();

	roi_end_main();

	for (i = 0; i < threads; i++) {
		err = pthread_join(thread_id[i], NULL);
		if (err) {
			perror("pthread_join");
			exit(1);
		}
	}

	return get_duration(benchmark.beg, benchmark.end);
}

/* same format for all benchmarks */
static inline void print_result(const char *name, uint64_t threads, double time, uint64_t arg) {
	printf("RESULT: [%s] #thread: %ld time(s): %10.6f arg: %ld\n",
				name, threads, time, arg);
}
#endif
