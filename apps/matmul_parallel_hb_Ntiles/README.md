# matmul_parallel_hb_Ntiles

Parallel HammerBlade matrix multiply with tile count set to `N`.

This app computes $C=A\times B$ in `kernel.cpp` using `N` active tiles
(one tile per matrix row/task partition), without a fixed block-size parameter.

## Files

- `main.cpp`: host-side setup, input generation (`PATTERN`), kernel launch, and correctness check.
- `kernel.cpp`: parallel kernel with `N`-tile work partitioning.
- `tests.mk`: test matrix for `N`, `NITER`, `PATTERN`, `warm-cache`.

## Build and run

1. `make generate`
2. `cd N_16__NITER_1__tile-x_16__tile-y_8__PATTERN_0__warm-cache_no`
3. `make profile.log`

## Notes

- Tile count is configured to match `N` for these experiments.
- Correctness is checked in `main.cpp` against a host reference (SSE threshold).
