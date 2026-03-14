# matmul_parallel_hb_128tiles

Parallel HammerBlade matrix multiply targeting full tile utilization.

This app computes $C=A\times B$ in `kernel.cpp` using a task/chunk mapping that
helps keep all tiles active (e.g., 128 tiles on a 16x8 pod) for selected `N`.

## Files

- `main.cpp`: host-side setup, input generation (`PATTERN`), kernel launch, and correctness check.
- `kernel.cpp`: parallel kernel with chunked row/column task assignment.
- `tests.mk`: test matrix for `N`, `NITER`, `PATTERN`, `warm-cache`.
- `parse_hb_stats.py`: convert HammerBlade stats into CSV.

## Build and run

1. `make generate`
2. `cd N_16__NITER_1__tile-x_16__tile-y_8__PATTERN_0__warm-cache_no`
3. `make profile.log`

## Notes

- The kernel currently includes a 1x4 register-blocked inner compute path.
- Correctness is checked in `main.cpp` against a host reference (SSE threshold).

