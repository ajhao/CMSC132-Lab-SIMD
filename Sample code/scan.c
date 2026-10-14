/* scan.c — CMSC 132 · Data-Level Parallelism lab (Task 4)
 *
 * An elementwise loop like the map in poly.c, but with ONE crucial change:
 * each iteration reads the result of the PREVIOUS iteration:
 *
 *        y[i] = a * y[i-1] + x[i];     <-- y[i-1] was just computed
 *
 * This is a LOOP-CARRIED DEPENDENCY. The lanes of a SIMD unit run at the
 * same time, but here iteration i cannot start until iteration i-1 has
 * finished — so there is nothing to do in parallel. Build this with the
 * VECTOR flags and read the vectorizer report: it should say NOT vectorized.
 *
 * Build:  make scan        Run:  ./scan
 */

#include <stdio.h>
#include <stdlib.h>
#include "timer.h"

typedef float real;

/* Dependent "scan" / recurrence — deliberately NOT vectorizable. */
void scan(long n, real a, const real *restrict x, real *restrict y) {
    for (long i = 1; i < n; i++)
        y[i] = a * y[i - 1] + x[i];   /* needs the previous result */
}

int main(int argc, char **argv) {
    long  n    = (argc > 1) ? atol(argv[1]) : 8L * 1024 * 1024;
    int   reps = (argc > 2) ? atoi(argv[2]) : 30;
    const real a = (real)0.5;   /* <1 keeps the recurrence from exploding */

    real *x = malloc((size_t)n * sizeof(real));
    real *y = malloc((size_t)n * sizeof(real));
    if (!x || !y) { fprintf(stderr, "allocation failed\n"); return 1; }

    for (long i = 0; i < n; i++) { x[i] = (real)1.0; y[i] = (real)(i & 7); }
    scan(n, a, x, y);  /* warm-up */

    double best_ms = 1e30;
    for (int r = 0; r < reps; r++) {
        double t0 = now_sec();
        scan(n, a, x, y);
        double ms = (now_sec() - t0) * 1000.0;
        if (ms < best_ms) best_ms = ms;
    }

    double checksum = 0.0;
    for (long i = 0; i < n; i++) checksum += (double)y[i];

    printf("N = %ld (dependent scan)\n", n);
    printf("best time : %.3f ms   (fastest of %d reps)\n", best_ms, reps);
    printf("checksum  : %.3e\n", checksum);

    free(x); free(y);
    return 0;
}
