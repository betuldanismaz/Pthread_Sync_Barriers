# Barrier Synchronization in Pthreads

A comprehensive implementation and comparison of two barrier synchronization techniques in C using POSIX threads: **busy-wait** and **condition variables**.

## Overview

This project demonstrates how to coordinate multiple threads at synchronization points using barriers. A barrier forces all participating threads to wait until every thread has reached the barrier before any can proceed. This is essential in parallel computing when phases of computation must complete before the next phase begins.

### Use Case

The demonstration program performs a two-phase parallel computation on an array:

1. **Phase 1**: Each thread squares its portion of the array
2. **Barrier**: All threads synchronize here
3. **Phase 2**: Each thread computes the sum of its squared values

Without the barrier, fast threads could start Phase 2 while slow threads are still in Phase 1, leading to incorrect results.

## Project Structure

```
barrier_synch/
├── src/
│   ├── barrier_api.h           # Common barrier interface
│   ├── barrier_busywait.c      # Busy-wait implementation
│   ├── barrier_condvar.c       # Condition variable implementation
│   ├── main_busywait.c         # Test program for busy-wait
│   ├── main_condvar.c          # Test program for condition variable
│   ├── common.h                # Shared definitions
│   └── common.c                # Utility functions
├── outputs/
│   ├── outputs_busywait.txt    # Results from busy-wait tests
│   └── outputs_condvar.txt     # Results from condition variable tests
├── Makefile
└── README.md
```

## Implementation Details

### 1. Busy-Wait Barrier (`barrier_busywait.c`)

**Mechanism**: Uses atomic operations and spin-waiting.

```c
struct barrier {
    int thread_count;           // Total threads required
    atomic_int arrived;         // Atomic counter
    pthread_mutex_t mutex;      // Protects critical section
};
```

**How it works**:

- Each thread atomically increments the `arrived` counter
- Threads spin in a loop, continuously checking if all threads have arrived
- Uses C11 atomic operations (`atomic_fetch_add_explicit`, `atomic_load_explicit`) with memory ordering guarantees

**Pros**:

- Low latency when wait times are short
- No context switching overhead

**Cons**:

- Wastes CPU cycles during spinning
- Poor performance when threads arrive at different times
- Can cause cache line contention

### 2. Condition Variable Barrier (`barrier_condvar.c`)

**Mechanism**: Uses pthread condition variables for efficient blocking.

```c
struct barrier {
    int thread_count;           // Total threads required
    int arrived;                // Counter (protected by mutex)
    int generation;             // Prevents spurious wakeups across reuses
    pthread_mutex_t mutex;      // Protects shared state
    pthread_cond_t cond;        // Condition variable for waiting
};
```

**How it works**:

- Each thread locks the mutex and increments `arrived`
- If not the last thread, it waits on the condition variable (releases mutex)
- The last thread resets the barrier, increments `generation`, and broadcasts to wake all waiting threads
- The generation counter prevents spurious wakeups when the barrier is reused

**Pros**:

- CPU-efficient: threads sleep instead of spinning
- Scales better with many threads
- Better for long wait times

**Cons**:

- Higher latency due to context switching
- More complex implementation

## Building the Project

### Prerequisites

- GCC compiler with C11 support
- POSIX threads library (pthread)
- Linux/Unix environment (or WSL on Windows)

### Compilation

```bash
# Build both implementations
make all

# Build only busy-wait version
make busywait

# Build only condition variable version
make condvar

# Clean build artifacts
make clean
```

## Running the Programs

Both programs accept a single argument: the number of threads to use.

```bash
# Run busy-wait implementation with 4 threads
./busywait 4

# Run condition variable implementation with 8 threads
./condvar 8
```

### Expected Output

```
Final sum of squares = 333833500
```

For `ARRAY_SIZE = 1000`, the expected result is:
$$\sum_{i=1}^{1000} i^2 = \frac{1000 \times 1001 \times 2001}{6} = 333,833,500$$

## Configuration

You can modify the array size in [`src/common.h`](src/common.h):

```c
#define ARRAY_SIZE 1000  // Change this value
```

## Performance Comparison

| Implementation         | Best Use Case            | CPU Usage       | Latency |
| ---------------------- | ------------------------ | --------------- | ------- |
| **Busy-Wait**          | Short waits, few threads | High (spinning) | Low     |
| **Condition Variable** | Long waits, many threads | Low (blocking)  | Higher  |

### When to Use Each

- **Busy-Wait**: Real-time systems, tight synchronization loops, when all threads arrive nearly simultaneously
- **Condition Variable**: General-purpose applications, when threads may arrive at significantly different times, when CPU efficiency matters

## Key Parallel Programming Concepts

### Race Conditions

Without proper synchronization, multiple threads accessing shared data can lead to undefined behavior. This project demonstrates:

- **Mutex protection** for the global sum
- **Atomic operations** for lock-free counter updates (busy-wait)
- **Condition variables** for efficient waiting

### Memory Ordering

The busy-wait implementation uses explicit memory ordering:

- `memory_order_release`: Ensures all previous writes are visible before the atomic increment
- `memory_order_acquire`: Ensures the read sees all writes from other threads

### Load Balancing

The `compute_range()` function distributes work evenly across threads, handling cases where the array size doesn't divide evenly.

## Testing

Sample outputs are provided in the `outputs/` directory:

- [`outputs_busywait.txt`](outputs/outputs_busywait.txt)
- [`outputs_condvar.txt`](outputs/outputs_condvar.txt)

Both should produce identical results, verifying correctness.

## Common Issues

### Compilation Errors

- **Missing `stdatomic.h`**: Ensure you're using GCC with C11 support (`-std=c11` or newer)
- **Undefined pthread functions**: Link with `-lpthread` flag

### Runtime Issues

- **Deadlock**: Usually indicates a bug in barrier logic (e.g., incorrect thread count)
- **Incorrect sum**: Suggests a race condition or barrier failure

## References

- POSIX Threads Programming: https://computing.llnl.gov/tutorials/pthreads/
- C11 Atomic Operations: ISO/IEC 9899:2011
- "The Art of Multiprocessor Programming" by Herlihy & Shavit

## License

This project is for educational purposes.

## Author

Developed as a demonstration of barrier synchronization techniques in parallel programming.
