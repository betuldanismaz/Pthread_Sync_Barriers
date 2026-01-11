#include "barrier_api.h"
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

struct barrier {
    int thread_count;  // total number of threads required to reach the barrier
    atomic_int arrived;       // how many threads have reached the barrier
    pthread_mutex_t mutex;  // mutex for protecting the barrier
};

barrier_t* barrier_create(int thread_count) {
    barrier_t* b = (barrier_t*)calloc(1, sizeof(barrier_t));
    if (!b) return NULL;

    b->thread_count = thread_count;
    atomic_init(&b->arrived, 0);

    if (pthread_mutex_init(&b->mutex, NULL) != 0) {
        free(b);
        return NULL;
    }
    return b;
}

void barrier_wait(barrier_t* b) {
    
    pthread_mutex_lock(&b->mutex);
    int cur = atomic_fetch_add_explicit(&b->arrived, 1, memory_order_release) + 1;
    pthread_mutex_unlock(&b->mutex);

    // busy-wait until all threads arrive
    if (cur < b->thread_count) {
        while (atomic_load_explicit(&b->arrived, memory_order_acquire) < b->thread_count) {
            // spin
        }
    }
}

void barrier_destroy(barrier_t* b) {
    if (!b) return;
    pthread_mutex_destroy(&b->mutex);
    free(b);
}