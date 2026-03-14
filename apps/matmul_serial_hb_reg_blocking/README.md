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

# Optimized variants
# variant 0 = simple, 1 = tiled (cache-friendly), 2 = tiled+OpenMP (parallel)
./host_baseline_opt 32 1 0 0   # simple
./host_baseline_opt 32 1 0 1   # tiled
./host_baseline_opt 32 1 0 2   # tiled+OpenMP
```

## Building the HammerBlade program

The HB build requires the HammerBlade toolchain (bsg_cadenv + manycore SDK).
When you run `make generate`, the top‑level `cadenv.mk` file looks for a
valid CAD environment (the `bsg_cadenv` script). If it cannot find it you will
see the warning and failure you reported:

```
BSG MAKE WARN: Couldn't find bsg_cadenv. User must configure CAD Environment.
make: *** No rule to make target 'app_path.mk' ... Stop.
```

To fix this:

1. **Locate or install the CAD environment.**  On systems with HammerBlade
   installed there is usually a script called `bsg_cadenv` somewhere under
   `/opt/bsdg/` or in your project tree.  You can search for it with:

   ```bash
   find $HOME -name bsg_cadenv 2>/dev/null
   ```

2. **Export or source the environment.**  Either set the `BSG_CADENV`
   variable to the directory containing the script and/or source it directly:

   ```bash
   export BSG_CADENV=/path/to/bsg_cadenv
   source $BSG_CADENV
   # (or simply: source /path/to/bsg_cadenv)
   ```

   After this the `make generate` command will be able to create `app_path.mk`
   and proceed with compilation.

3. **Helper checker script.**  To make this easier the repository includes a
   small helper script you can run before attempting to build:

   ```bash
   ./check_cadenv.sh
   ```

   It will print whether the environment is configured and give instructions.

If you don’t have a CAD environment installed, talk to your system
administrator or the HammerBlade documentation team – it’s part of the
HammerBlade SDK and is normally provided with the FPGA/ASIC build tools.

Once the environment is set up the normal workflow is:

```bash
cd matmul_serial_hb
make generate   # will succeed now
```

