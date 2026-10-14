/* timer.h — minimal monotonic wall-clock timer for CMSC 132 labs.
 *
 * Usage:
 *     double t0 = now_sec();
 *     ... work ...
 *     double elapsed_ms = (now_sec() - t0) * 1000.0;
 *
 * Portable across Linux and macOS (x86-64 and Apple Silicon/ARM).
 * No dependencies beyond the C standard library + POSIX clock_gettime.
 */
#ifndef CMSC132_TIMER_H
#define CMSC132_TIMER_H

#include <time.h>

/* Returns the current time in SECONDS from a monotonic clock.
 * Monotonic = never jumps backward (immune to NTP / clock changes),
 * which is what you want for measuring elapsed time. */
static inline double now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

#endif /* CMSC132_TIMER_H */
