# matmul_serial_hb

This directory contains serial and simple-optimized matrix multiply baselines for HammerBlade using loop unrolling.




# matmul_serial_hb_unrolling

Single-core HammerBlade matrix multiply with loop unrolling.

This app computes $C=A\times B$ in `kernel.cpp` using manual unrolling by 8 on
the inner `z` loop (`z += 8`), plus a remainder loop.

## Files

- `main.cpp`: host-side setup, input generation (`PATTERN`), kernel launch, and correctness check.
- `kernel.cpp`: unrolled device kernel.
- `tests.mk`: test matrix for `N`, `NITER`, `PATTERN`, `warm-cache`.
- `parse_hb_stats.py`: helper to convert HammerBlade stats into CSV.

## Input patterns

- `PATTERN=0`: deterministic small integers.
- `PATTERN=1`: deterministic pseudo-random values in `[-1, 1)`.
- `PATTERN=2`: sparse/structured values.

## Build and run

1. `make generate`
2. `cd N_16__NITER_1__tile-x_16__tile-y_8__PATTERN_0__warm-cache_no`
3. `make profile.log`

## Correctness

Validation is done in `main.cpp` against a host reference implementation
using SSE.
- `main.cpp`: host-side setup, input generation (`PATTERN`), kernel launch, and correctness check.

- `kernel.cpp`: unrolled device kernel (`z += 8`).

- `tests.mk`: test matrix for `N`, `NITER`, `PATTERN`, `warm-cache`.
