#ifndef BARRIER_API_H
#define BARRIER_API_H

// A minimal barrier API used by both implementations
typedef struct barrier barrier_t;

barrier_t* barrier_create(int thread_count);
void barrier_wait(barrier_t* b);
void barrier_destroy(barrier_t* b);

#endif
