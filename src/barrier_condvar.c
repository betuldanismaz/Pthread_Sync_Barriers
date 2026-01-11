#include "barrier_api.h"
#include <pthread.h>
#include <stdlib.h>

struct barrier {
    int thread_count;
    int arrived;
    int generation;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
};

barrier_t* barrier_create(int thread_count) {
    barrier_t* b = (barrier_t*)calloc(1, sizeof(barrier_t));
    if (!b) return NULL;

    b->thread_count = thread_count;
    b->arrived = 0;
    b->generation = 0;

    if (pthread_mutex_init(&b->mutex, NULL) != 0) {
        free(b);
        return NULL;
    }
    if (pthread_cond_init(&b->cond, NULL) != 0) {
        pthread_mutex_destroy(&b->mutex);
        free(b);
        return NULL;
    }
    return b;
}

void barrier_wait(barrier_t* b) {
    pthread_mutex_lock(&b->mutex);

    int my_gen = b->generation;
    b->arrived++;

    if (b->arrived == b->thread_count) {
        // Last thread arrives: reset for potential reuse
        b->arrived = 0;
        b->generation++;
        pthread_cond_broadcast(&b->cond);
        pthread_mutex_unlock(&b->mutex);
        return;
    }

    while (my_gen == b->generation) {
        pthread_cond_wait(&b->cond, &b->mutex);
    }

    pthread_mutex_unlock(&b->mutex);
}

void barrier_destroy(barrier_t* b) {
    if (!b) return;
    pthread_cond_destroy(&b->cond);
    pthread_mutex_destroy(&b->mutex);
    free(b);
}
