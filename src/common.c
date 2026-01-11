#include "common.h"

void compute_range(size_t n, int thread_id, int thread_count, size_t* start, size_t* end) {
    // Block distribution with remainder:
    // base = n / T, rem = n % T
    // first 'rem' threads get (base+1), others get base
    size_t base = n / (size_t)thread_count;
    size_t rem  = n % (size_t)thread_count;

    size_t s = 0;

    if ((size_t)thread_id < rem) {
        s = (size_t)thread_id * (base + 1);
        *start = s;
        *end   = s + (base + 1);
    } else {
        s = rem * (base + 1) + ((size_t)thread_id - rem) * base;
        *start = s;
        *end   = s + base;
    }
}
