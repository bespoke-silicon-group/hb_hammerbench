# matmul_serial_hb_reg_blocking

Single-core HammerBlade matrix multiply with register blocking (**most optimized serial method** in this project).

This app computes $C=A\times B$ in `kernel.cpp` using a 1x4 register block in
the `x` dimension (four output columns accumulated per inner-loop iteration).

## Files

- `main.cpp`: host-side setup, input generation (`PATTERN`), kernel launch, and correctness check.
- `kernel.cpp`: 1x4 register-blocked device kernel.
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




