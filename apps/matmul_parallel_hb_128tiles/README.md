# matmul_parallel_hb

This application implements a tiled parallel matrix multiply on HammerBlade.

* `main.cpp` – host driver that allocates matrices, copies data, launches the kernel, and validates results.
* `kernel.cpp` – tile-based kernel where each tile (core) computes one `BLOCK_DIM`×`BLOCK_DIM` submatrix of the result.
* `Makefile`, `tests.mk` – same pattern as other hb_hammerbench apps.
* `parse_hb_stats.py` – helper to convert HB runtime stat output into CSV for analysis.

`BLOCK_DIM` is currently fixed at 16; you can change it to tune for the local memory size.

## Building & running
As with the serial app, you need a configured HammerBlade CAD environment; see `../matmul_serial_hb/README.md` for instructions.

## Optimization notes
1. **Tile size** – `BLOCK_DIM` should be chosen so that two blocks (A and B) plus C fit in local scratch memory.
2. **Load balance** – distribute blocks evenly over `bsg_tiles_X` × `bsg_tiles_Y`; current loop strides by tile counts.
3. **DMA** – the current version does simple element loads; a performance version should use DMA transfers and double-buffer.
4. **Loop unrolling** – inner multiply loops can be unrolled or use `fmaf` to utilize hardware.
5. **Prefetching/overlap** – schedule next A/B block load while computing current partial product.

This app will serve as the starting point for iterative optimization (see project README).
