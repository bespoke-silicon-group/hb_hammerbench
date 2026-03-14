# GEMM_dual_DMEM

HammerBlade GEMM app using dual-DMEM staging (A-row + B-col) for `N=32` with tile-group `8x4` (32 cores).

## Files
- `main.c`: host launcher + correctness check
- `kernel.cpp`: device kernel implementation
- `Makefile`, `template.mk`, `tests.mk`: hb_hammerbench test generation/build flow

## Default test
`tests.mk` defines:
- `N=32`
- `TGX=8`
- `TGY=4`

## Build
```bash
cd apps/GEMM_dual_DMEM
make
cd N_32__TGX_8__TGY_4
make main.so
make main.riscv
```

## Run
```bash
cd apps/GEMM_dual_DMEM/N_32__TGX_8__TGY_4
make exec.log
```

## Reference result (from this configuration)
- Cycles: `37,995`
- Throughput: `~1.725 GFLOPS`
- Stall mix (`profile`):
  - `stall_lr_aq`: `85.01%`
  - `stall_depend_dram_load`: `13.64%`
  - load miss: `0.0058%`

## Contact
For additional data or profiling results, please contact [ericpp-peng](https://github.com/ericpp-peng).
