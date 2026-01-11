#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#include "common.h"
#include "barrier_api.h"

static void* worker(void* argp) {
    thread_arg_t* arg = (thread_arg_t*)argp;

    // Phase 1: A[i] = A[i] * A[i]
    for (size_t i = arg->start; i < arg->end; i++) {
        long long x = arg->A[i];
        arg->A[i] = (int)(x * x);
    }

    // Barrier: all threads must finish Phase 1 before Phase 2
    barrier_wait((barrier_t*)arg->barrier);

    // Phase 2: local sum
    long long sum = 0;
    for (size_t i = arg->start; i < arg->end; i++) {
        sum += arg->A[i];
    }
    arg->local_sum = sum;

    // Combine into global sum (protected)
    pthread_mutex_lock(arg->sum_mutex);
    *(arg->global_sum) += sum;
    pthread_mutex_unlock(arg->sum_mutex);

    return NULL;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <thread_count>\n", argv[0]);
        return 1;
    }

    char* endp = NULL;
    long t = strtol(argv[1], &endp, 10);
    if (endp == argv[1] || *endp != '\0' || t <= 0) {
        fprintf(stderr, "Invalid thread_count: %s\n", argv[1]);
        return 1;
    }
    int thread_count = (int)t;

    // Allocate and init A
    int* A = (int*)malloc(sizeof(int) * ARRAY_SIZE);
    if (!A) {
        perror("malloc A");
        return 1;
    }
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        A[i] = (int)(i + 1);
    }

    // Shared sum + mutex
    long long global_sum = 0;
    pthread_mutex_t sum_mutex;
    pthread_mutex_init(&sum_mutex, NULL);

    // Barrier
    barrier_t* barrier = barrier_create(thread_count);
    if (!barrier) {
        fprintf(stderr, "Failed to create barrier\n");
        free(A);
        return 1;
    }

    pthread_t* threads = (pthread_t*)malloc(sizeof(pthread_t) * (size_t)thread_count);
    thread_arg_t* args = (thread_arg_t*)malloc(sizeof(thread_arg_t) * (size_t)thread_count);
    if (!threads || !args) {
        perror("malloc threads/args");
        barrier_destroy(barrier);
        pthread_mutex_destroy(&sum_mutex);
        free(A);
        free(threads);
        free(args);
        return 1;
    }

    // Launch threads
    for (int tid = 0; tid < thread_count; tid++) {
        size_t start = 0, end = 0;
        compute_range(ARRAY_SIZE, tid, thread_count, &start, &end);

        args[tid].thread_id = tid;
        args[tid].thread_count = thread_count;
        args[tid].A = A;
        args[tid].n = ARRAY_SIZE;
        args[tid].start = start;
        args[tid].end = end;
        args[tid].local_sum = 0;
        args[tid].global_sum = &global_sum;
        args[tid].sum_mutex = &sum_mutex;
        args[tid].barrier = barrier;

        int rc = pthread_create(&threads[tid], NULL, worker, &args[tid]);
        if (rc != 0) {
            errno = rc;
            perror("pthread_create");
            // join what we started
            for (int j = 0; j < tid; j++) pthread_join(threads[j], NULL);
            free(threads);
            free(args);
            barrier_destroy(barrier);
            pthread_mutex_destroy(&sum_mutex);
            free(A);
            return 1;
        }
    }

    for (int tid = 0; tid < thread_count; tid++) {
        pthread_join(threads[tid], NULL);
    }

    printf("Final sum of squares = %lld\n", global_sum);

    free(threads);
    free(args);
    barrier_destroy(barrier);
    pthread_mutex_destroy(&sum_mutex);
    free(A);

    return 0;
}
