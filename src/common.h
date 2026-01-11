#ifndef COMMON_H
#define COMMON_H

#include <pthread.h>
#include <stddef.h>

#define ARRAY_SIZE 1000

typedef struct {
    int thread_id;
    int thread_count;

    int* A;
    size_t n;

    // Range [start, end) for this thread
    size_t start;
    size_t end;

    // For Phase 2
    long long local_sum;

    // Shared global sum (protected by a mutex in Phase 2)
    long long* global_sum;
    pthread_mutex_t* sum_mutex;

    // Barrier handle (opaque pointers; will be defined per implementation)
    void* barrier;
} thread_arg_t;

// Partition [0, n) into thread_count contiguous blocks.
// This returns [start, end) for thread_id, balanced as evenly as possible.
void compute_range(size_t n, int thread_id, int thread_count, size_t* start, size_t* end);

#endif
