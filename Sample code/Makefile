# CMSC 132 — Data-Level Parallelism (SIMD) lab
# Auto-detects gcc vs clang and x86-64 vs Apple Silicon/ARM.
#
#   make scalar   # Task 1: SIMD OFF
#   make vector   # Tasks 1-3: SIMD ON (prints the vectorizer report)
#   make scan     # Task 4: dependent loop (report should say NOT vectorized)
#   make all      # build all three
#   make clean    # remove the binaries

CC    ?= cc
ARCH  := $(shell uname -m)
CLANG := $(shell $(CC) --version 2>/dev/null | grep -ci clang)

# -march=native on x86; -mcpu=native is the reliable form on Apple Silicon/ARM.
ifeq ($(filter arm64 aarch64,$(ARCH)),)
  ARCHFLAG := -march=native
else
  ARCHFLAG := -mcpu=native
endif

# Per-compiler flags: how to DISABLE vectorization, and how to print the REPORT.
ifeq ($(CLANG),0)
  # ---- GCC ----
  NOVEC  := -fno-tree-vectorize
  REPORT := -fopt-info-vec -fopt-info-vec-missed
else
  # ---- Clang / Apple clang ----
  NOVEC  := -fno-vectorize -fno-slp-vectorize
  REPORT := -Rpass=loop-vectorize -Rpass-missed=loop-vectorize
endif

COMMON := -std=c11 -Wall

.PHONY: all clean
all: scalar vector scan

# Task 1: baseline — SIMD turned off, one element per instruction.
scalar: poly.c timer.h
	$(CC) $(COMMON) -O2 $(NOVEC) poly.c -o scalar

# Tasks 1-3: SIMD on. The REPORT flags make the compiler tell you, at build
# time, which loops it vectorized. Look for the poly() loop being vectorized.
vector: poly.c timer.h
	$(CC) $(COMMON) -O3 $(ARCHFLAG) $(REPORT) poly.c -o vector

# Task 4: same flags as vector, but the dependent loop cannot vectorize.
# The report should say the loop was NOT vectorized (and give a reason).
scan: scan.c timer.h
	$(CC) $(COMMON) -O3 $(ARCHFLAG) $(REPORT) scan.c -o scan

clean:
	rm -f scalar vector scan
