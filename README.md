# CMSC 132: Data-Level Parallelism Lab
## Seeing SIMD: One Loop, Many Lanes

**Unit:** Parallelism (Data-Level Parallelism)
**Tools:** a C compiler (`gcc` or `clang`) and a terminal. Any laptop, x86-64 **or** Apple Silicon/ARM. No simulator, no special hardware.

---

### Introduction
A modern CPU core can apply **one instruction to several data elements at once**, this is **SIMD** (Single Instruction, Multiple Data), the hardware behind *data-level parallelism*. In this lab you take **one unchanged loop**, let the compiler turn it into SIMD, and *measure* the speedup. Then you discover two things every architect must know: **wider data types get fewer lanes**, and **loop-carried dependencies kill vectorization entirely**.

### Learning outcomes
By the end you can:
1. Explain SIMD as *one instruction, many lanes*, and estimate the ideal speedup from the lane count.
2. Turn on and **verify** compiler autovectorization, in the build report *and* in the assembly.
3. Explain why **`double` is slower than `float`** under SIMD (half as many lanes per register).
4. Recognize a **loop-carried dependency** and explain why it cannot be vectorized.

### Prerequisites
S14 (vector/SIMD concept, lanes). Comfort building and running a C program from the terminal.

---

## What's in your folder (`Sample code/`)
| File | Role |
|---|---|
| `poly.c` | The experiment (Tasks 0–3): an elementwise **map** `y[i] = P(x[i])`, the same degree-15 polynomial per element. Independent iterations → a SIMD target. |
| `scan.c` | Task 4: the **same shape but with a dependency**: `y[i] = a*y[i-1] + x[i]`. |
| `timer.h` | A tiny monotonic wall-clock timer. |
| `Makefile` | Builds `scalar`, `vector`, and `scan`. Auto-detects gcc/clang and x86/ARM. |

> **Why a polynomial and not something like `y=a*x+y`?** A trivial loop is limited by **memory bandwidth**, not by how many lanes you have, so it would *hide* the SIMD effect. The polynomial does real arithmetic per element, so the vector width shows up clearly. (Remember the "bandwidth wall"?)

---

## Setup (do this first)
Open a terminal in the `Sample code/` folder and build all three programs:
```
make all
```
This produces `./scalar`, `./vector`, and `./scan`. If `make all` succeeds, you're ready.

Two build modes matter:
- **`scalar`** is compiled with SIMD **disabled** - one element per instruction (your baseline).
- **`vector`** is compiled with `-O3 -march=native` (or `-mcpu=native` on ARM) , SIMD **enabled**.

The loop in `poly.c` is **identical** in both. The only difference is whether the compiler is allowed to use vector lanes.

---

## Task 0: Warm-up *(≈ 20 min)*
Run the baseline and the SIMD build once:
```
./scalar
./vector
```
Each prints a **best time** (fastest of several runs) and a **checksum**.

**Record:** both times. **Check:** the two **checksums are identical**, SIMD must compute the *same* answer, just faster.
**Deliverable (0):** the two lines of output, and one sentence: you changed *no* source code between them, so where did the time difference come from?

> *Tip:* close other heavy apps and keep the laptop plugged in, background work and thermal throttling add noise. The programs already report the **fastest** run to cancel most of that out.

---

## Task 1: Measure the speedup *(≈ 25 min)*
Run each program **3 times** and take the best time you see for each.
Compute: **speedup = scalar_time ÷ vector_time**.

**Deliverable (1):** a small table 

| build | best time (ms) | speedup |
|---|---|---|
| scalar | … | 1.0× |
| vector | … | **…×** |

plus one sentence naming *what the hardware did* to earn that speedup.

---

## Task 2: Prove it actually vectorized *(≈ 40 min)*
A speedup is only convincing if you can *see* the vector instructions. Two pieces of evidence:

**(a) The compiler's report.** Rebuild the vector program and read the messages the compiler prints:
```
make clean
make vector
```
Look for a line about the loop in **`poly()`** being **vectorized** (it names the file and line, e.g. `poly.c:43 ... vectorized loop (vectorization width: N)`). The **width** is how many elements share one instruction.

**(b) The assembly.** Look at the actual instructions:
- **macOS:** `otool -tV vector | grep -iE 'fmla|\.4s|\.2d'`
- **Linux:** `objdump -d vector | grep -E 'fmla|mulps|vfmadd'`

Vector ops carry a **lane marker** - on ARM a `.4s`/`.2d` suffix or a `v` register; on x86 a `p`acked mnemonic like `mulps`/`vfmadd…ps`. Now do the same on `scalar` and you'll see only **single**-element ops (ARM `s`/`d` registers; x86 `…ss`/`…sd`).

**Deliverable (2):** paste **one** "vectorized loop" report line **and** one vector-instruction line from the assembly, and state the **vectorization width** the compiler chose.

---

## Task 3: Lanes vs. data type *(≈ 40 min)*
A vector register is a fixed number of **bits**. So it holds **twice as many `float`s as `double`s** (e.g. 4 floats vs 2 doubles on ARM NEON; 8 vs 4 on x86 AVX). Fewer lanes → less speedup.

1. **Predict first (write it down):** if you switch from `float` to `double`, what should happen to the vector speedup?
2. Edit **one line** in `poly.c`, the line marked `<<< TASK 3 >>>`:
   ```c
   typedef float real;      →      typedef double real;
   ```
3. Rebuild and re-measure:
   ```
   make clean
   make scalar
   make vector
   ./scalar
   ./vector
   ```

**Deliverable (3):** your written prediction, then a table with the **float** and **double** vector speedups, and the ratio of the two **vector** times (float-vector vs double-vector). Explain the ratio using **lane count**. *(Change the line back to `float` when done.)*

> Note: the **scalar** times for float and double are about the **same** - the scalar core does one element at a time either way. The difference appears **only** with vectors. That's the whole point.

---

## Task 4: Break the vectorizer *(≈ 40 min)*
Vectorization needs the iterations to be **independent**. `scan.c` deliberately makes each iteration depend on the previous one:
```c
y[i] = a * y[i-1] + x[i];   // needs the result of iteration i-1
```
Build it with the **vector** flags and read the report:
```
make clean
make scan
```

**Deliverable (4):** paste the compiler line about the loop in **`scan()`** (it will say the loop was **not vectorized**), and explain in **two sentences** why SIMD lanes can't help here, even though the loop looks almost identical to the one in `poly.c`.

---

## Wrap-up write-up *(≈ 15 min)*
Answer briefly, in your own words:
1. You changed no source between `scalar` and `vector` in Task 1 what gave the speedup?
2. How did you *verify* (not just assume) that vectorization happened?
3. Why was `double` slower than `float` under SIMD?
4. Why wouldn't the `scan` loop vectorize, and what general property must a loop have to be SIMD-friendly?

---

## Submission
Submit a single PDF/`.md` containing: the five deliverables (Tasks 0–4), your wrap-up answers, and **your machine** (OS + CPU; e.g. `uname -m` / "Apple M-series" / "Intel/AMD x86-64"). Report *your* numbers, they will differ by machine, and that's expected.

## Grading (20 pts)
| Item | Pts |
|---|---|
| Task 1: speedup measured correctly (best-of runs; checksums match) | 25 |
| Task 2: vectorization verified: report **and** assembly, width stated | 25 |
| Task 3: prediction + float-vs-double table + lane-count explanation | 25 |
| Task 4: dependency identified + correct reasoning | 15 |
| Wrap-up answers | 10 |


---
