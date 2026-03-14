# matmul_serial_hb

This directory contains serial and simple-optimized matrix multiply baselines for HammerBlade.

## Files

- `main.cpp` / `kernel.cpp` – HammerBlade host+device serial kernel (single-core).
- `host_baseline.c` – x86 simple row-major baseline with timing and correctness checks.
- `host_baseline_optimized.c` – x86 optimized baselines: tiled, tiled+OpenMP.
- `Makefile.host` – build helper for both x86 baselines.
- `run_host.sh` – convenience script to run the simple baseline.
- `parse_hb_stats.py` – helper to convert HB runtime stat output into CSV.

## Running the x86 baselines locally (no HammerBlade required)

```bash
cd examples/hb_hammerbench/apps/matmul_serial_hb
make -f Makefile.host all

# Simple baseline (row-major)
./host_baseline 32 1 0
# matmul_serial_hb_baseline

Single-core HammerBlade matrix multiply baseline (**no optimization**).

This app implements the basic method $C=A\times B$ in `kernel.cpp` using a plain
triple loop over `y`, `x`, and `z`:

- no loop unrolling
- no register blocking
- no tiling optimization

## Files

- `main.cpp`: host-side setup, input generation (`PATTERN`), kernel launch, and correctness check.
- `kernel.cpp`: baseline device kernel.
- `tests.mk`: test matrix for `N`, `NITER`, `PATTERN`, `warm-cache`.
- `parse_hb_stats.py`: helper to convert HammerBlade stats into CSV.

## Input patterns

- `PATTERN=0`: deterministic small integers (`i%7`, `i%3`).
- `PATTERN=1`: deterministic pseudo-random values in `[-1, 1)`.
- `PATTERN=2`: sparse/structured pattern with many zeros.

## Build and run

1. `make generate`
2. `cd N_16__NITER_1__tile-x_16__tile-y_8__PATTERN_0__warm-cache_no`
3. `make profile.log`


