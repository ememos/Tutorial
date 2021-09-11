#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <time.h>
#include <locale.h>
#include <sys/sysinfo.h>
#include <sys/wait.h>

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

static struct benchmark *bp = &benchmark;
#ifdef MP
static int bpid;
#endif

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

#ifdef MP
      bpid = shmget(IPC_PRIVATE, sizeof(struct benchmark), IPC_CREAT | 0644);
      if (bpid < 0) {
              perror("shmget");
              return -1;
      }
      void *mem = shmat(bpid, NULL, 0);
      if (mem == (void *)-1) {
              perror("shmat");
              return -1;
      }
      bp = (struct benchmark *)mem;
      memset(bp, 0, sizeof(struct benchmark));

      pthread_barrierattr_t brattr;
      pthread_barrierattr_init(&brattr);
      pthread_barrierattr_setpshared(&brattr, PTHREAD_PROCESS_SHARED);
#endif
      bp->threads = threads;
      for (i = 0; i < 4; i++) {
#ifdef MP
              ret = pthread_barrier_init(&bp->barriers[i], &brattr, threads + 1);
#else
              ret = pthread_barrier_init(&bp->barriers[i], NULL, threads + 1);
#endif
		if (ret) {
			fprintf(stderr, "failed to init barriers\n");
			return ret;
		}
	}
	bp->cmd_roi_before = getenv("ROI_BEFORE");
	bp->cmd_roi_after = getenv("ROI_AFTER");
	bp->init = 1;

	return 0;
}

static inline struct tdata **init_tdata_numa() {
	struct tdata **ret;
	int i;
	int node;
	int core_id;

	if (!bp->init) {
		fprintf(stderr, "ERROR: you should call init_benchmark() first.\n");
		exit(-1);
	}

	node = numa_node_of_cpu(0); /* on main thread */
	ret = (struct tdata **) numa_alloc_onnode(bp->threads * sizeof(struct tdata *), node);
	if (!ret) {
		fprintf(stderr, "ERROR: failed to allocate tdata\n");
		return NULL;
	}

	for (i = 0; i < bp->threads; ++i) {
		core_id = i % num_proc();
		node = numa_node_of_cpu(core_id);
		ret[i] = (struct tdata *) numa_alloc_onnode(sizeof(struct tdata), node);
		if (!ret[i]) goto error;
		ret[i]->tid = i;
		ret[i]->data = NULL;
	}

	return ret;

error:
	for (i = 0; i < bp->threads; ++i) {
		if (ret[i])
			numa_free(ret[i], sizeof(struct tdata));
	}
	numa_free(ret, bp->threads * sizeof(struct tdata *));
	return NULL;
}

static inline struct tdata **init_tdata() {
	struct tdata **ret;
	int i;

	if (!bp->init) {
		fprintf(stderr, "ERROR: you should call init_benchmark() first.\n");
		exit(-1);
	}

	ret = (struct tdata **) calloc(bp->threads, sizeof(struct tdata *));
	if (!ret) {
		fprintf(stderr, "ERROR: failed to allocate tdata\n");
		return NULL;
	}

	for (i = 0; i < bp->threads; ++i) {
		ret[i] = (struct tdata *) malloc(sizeof(struct tdata));
		if (!ret[i]) goto error;
		ret[i]->tid = i;
		ret[i]->data = NULL;
	}

	return ret;

error:
	for (i = 0; i < bp->threads; ++i) {
		if (ret[i])
			free(ret[i]);
	}
	free(ret);
	return NULL;
}

static inline void free_tdata_numa(struct tdata **tdata) {
	int i;

	if (!bp->init) {
		fprintf(stderr, "ERROR: you should call init_benchmark() first.\n");
		exit(-1);
	}

	for (i = 0; i < bp->threads; ++i)
		numa_free(tdata[i], sizeof(struct tdata));
	numa_free(tdata, bp->threads * sizeof(struct tdata *));
}

static inline void free_tdata(struct tdata **tdata) {
	int i;

	if (!bp->init) {
		fprintf(stderr, "ERROR: you should call init_benchmark() first.\n");
		exit(-1);
	}

	for (i = 0; i < benchmark.threads; ++i)
		free(tdata[i]);
	free(tdata);
}

static inline void roi_begin_main() {
	if (!bp->init) {
		fprintf(stderr, "ERROR: you should call init_benchmark() first.\n");
		exit(-1);
	}
	pthread_barrier_wait(&bp->barriers[0]);
	sleep(1); /* wait for sub-threads stuck at the second barrier. */
	fprintf(stderr, "[INFO] ROI begins\n");
      if (bp->cmd_roi_before) {
		if (system(bp->cmd_roi_before))
			fprintf(stderr, "[INFO] ROI before command exit with non-zero status\n");
	}
      get_time(bp->beg);
	pthread_barrier_wait(&bp->barriers[1]);
}

static inline void roi_end_main() {
      pthread_barrier_wait(&bp->barriers[2]);
	get_time(bp->end);
	if (bp->cmd_roi_after) {
		if (system(bp->cmd_roi_after))
			fprintf(stderr, "[INFO] ROI after command exit with non-zero status\n");
	}
	fprintf(stderr, "[INFO] ROI ends\n");
	pthread_barrier_wait(&bp->barriers[3]);
}

static inline void roi_begin() {
	pthread_barrier_wait(&bp->barriers[0]);
	pthread_barrier_wait(&bp->barriers[1]);
}

static inline void roi_end() {
      pthread_barrier_wait(&bp->barriers[2]);
	sleep(1); /* do not disturb get_time of main thread */
	pthread_barrier_wait(&bp->barriers[3]);
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
	int i;
	uint64_t threads = bp->threads;

#ifdef MP
      int pid;
      for (i = 0; i < threads; i++) {
              pid = fork();
              if (pid == -1) {
                    perror("fork");
                    exit(1);
              } else if (pid == 0) { /* child */
                    func((void *)tdata[i]);
                    shmdt(bp);
                    exit(0);
              }
      }
#else
	pthread_t thread_id[threads];
	for (i = 0; i < threads; i++) {
		int err = pthread_create(&thread_id[i], NULL, func, (void *) tdata[i]);
		if (err) {
			perror("pthread_create");
			exit(1);
		}
      }
#endif

	roi_begin_main();
	roi_end_main();

#ifdef MP
      for (i = 0; i < threads; i++) {
            wait(NULL);
      }
#else
	for (i = 0; i < threads; i++) {
		int err = pthread_join(thread_id[i], NULL);
		if (err) {
			perror("pthread_join");
			exit(1);
		}
	}
#endif

	return get_duration(bp->beg, bp->end);
}

/* same format for all benchmarks */
static inline void print_result(const char *name, uint64_t threads, double time, uint64_t arg) {
	printf("RESULT: [%s] #thread: %ld time(s): %10.6f arg: %ld\n",
				name, threads, time, arg);
}
#endif
