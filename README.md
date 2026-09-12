# The HammerBlade Benchmark Suite

HammerBench is a collection of parallel benchmarks ported for HammerBlade RISC-V Manycore. 


## How to Use This Repository


### Install
This repository is meant to be cloned into [bsg_replicant](https://github.com/bespoke-silicon-group/bsg_replicant) which is meant to be cloned into [bsg_bladerunner](https://github.com/bespoke-silicon-group/bsg_bladerunner).

For initial setup do the following:
1. Clone [bsg_bladerunner](https://github.com/bespoke-silicon-group/bsg_bladerunner) and follow the setup instructions.
2. Initialize Bladerunner's recursive submodules, which include this repository.

For a fresh macOS setup and the complete 34-case physical 16×8 GCC/LLVM
comparison, see [the compiler-comparison guide](docs/compiler-comparison.md).


### Running a benchmark
- `apps/` directory contains all the benchmarks.
- Go into one of the benchmarks (`apps/sgemm`), and run `make generate`. This will generate some launch directories (e.g. N_512__NITER_2).
- From the launch directory, run `make profile.log`. This will launch the simulation and generate profiling data.

### Compiling a device kernel with HammerBlade LLVM

The default device toolchain remains GCC. For current LLVM work, use the
HammerBlade default [`hammerblade-llvm22-integration` branch](https://github.com/bespoke-silicon-group/llvm-project/tree/hammerblade-llvm22-integration)
(LLVM 22.1.8; merged revision `0ee3b2946133808704dee5ca0b5ea12601454068`).
The SDK must include the LLVM support merged into
[`bsg_manycore` at `abb299058b63`](https://github.com/bespoke-silicon-group/bsg_manycore/commit/abb299058b63b9d907f997a74e64d2988d52119e)
or a compatible descendant; this supplies the no-return
atomic helper used by BFS and the compiler-support definitions.

From a fresh generated launch directory, select the opt-in fragment and a
build/install prefix containing `bin/clang`, `bin/clang++`, `bin/opt`,
`bin/llc`, `bin/llvm-objcopy`, and Clang's builtin resource headers:

```sh
gmake -f Makefile -f /path/to/hb_hammerbench/mk/llvm-hammerblade.mk \
  SHELL=/bin/bash '.SHELLFLAGS=-e -o pipefail -c' \
  RISCV_LLVM_PATH=/path/to/llvm22-build \
  RISCV_LLVM_OBJECT_OUTPUT=1 \
  RISCV_LLVM_OPT_FLAGS=-enable-dfa-jump-thread \
  main.so main.riscv
```

This path emits RV32 IR without running Clang's optimization passes, runs one
explicit `opt` pipeline, and uses `llc -mcpu=hb-rv32 -target-abi=ilp32f` to
produce an ELF object. LLVM `llvm-objcopy` removes debug information (including
relocations unsupported by the installed GNU binutils 2.32) and
`.riscv.attributes` (whose modern spelling the old linker cannot parse).
The GNU/newlib toolchain still builds the device runtime and performs the
final ELF link; host compilation is unchanged. The fragment preserves the
application's source optimization/numerical flags; comparisons must record
and match the effective settings. Omitting the fragment preserves GCC.

Use separate writable application/runtime build products when comparing
compilers. Make does not automatically rebuild an existing object when only
the compiler or flags change. Keep simulator reuse separate from device
compiler selection; this fragment does not select a hardware configuration.

The older LLVM 10 implementation is preserved on
[`archive/hammerblade-llvm10-macos`](https://github.com/bespoke-silicon-group/llvm-project/tree/archive/hammerblade-llvm10-macos)
at `ca69171a7ec8`. Its historical Clang/llc/assembly path remains available by
omitting `RISCV_LLVM_OBJECT_OUTPUT=1`; it is not the current LLVM 22 path.


### Downloading sparse graph datasets
- `apps/graph_data` contains README on how to download and extract sparse graphs (.mtx).
- Currently, bfs, pagerank, and spgemm use these sparse graphs.
- These benchmarks have READMEs and makefile targets on how to preprocess the sparse graph.
