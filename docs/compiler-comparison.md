# Build and run the complete stock 16×8 suite with GCC or LLVM

First build the GNU SDK, LLVM 22 tools and shared physical 16×8 Verilator
model, and set `REPLICANT_PATH`, `BSG_PLATFORM`, `BSG_MACHINE_PATH`,
`VERILATOR_ROOT` and `VERILATOR_THREADS=1` as described in Bladerunner's
`docs/macos-hammerbench.md`. Use GNU Make and Python 3. Set `HB` to the
absolute path of this application checkout/worktree. Use separate application
worktrees for GCC and LLVM; do not reuse one compiler's generated objects.

## Prepare the checked-in test matrix

```sh
git -C "$HB" -c url.https://github.com/.insteadOf=git@github.com: submodule update --init --recursive
gmake -C "$HB/apps" generate
gmake -C "$HB/apps/graph_data" -j3 wiki-Vote.mtx offshore.mtx soc-Pokec.mtx \
  ljournal-2008.mtx hollywood-2009.mtx roadNet-CA.mtx road-central.mtx road-usa.mtx
for graph in soc-Pokec hollywood-2009 ljournal-2008 roadNet-CA road-central road-usa; do
  gmake -C "$HB/apps/bfs/inputs" GRAPH_NAME="$graph" preprocess || exit 1
done
for graph in soc-Pokec hollywood-2009 ljournal-2008 roadNet-CA road-central road-usa wiki-Vote; do
  gmake -C "$HB/apps/pagerank/inputs" GRAPH_NAME="$graph" preprocess || exit 1
done
for graph in wiki-Vote offshore roadNet-CA road-central; do
  gmake -C "$HB/apps/spgemm/inputs" GRAPH_NAME="$graph" preprocess || exit 1
done
```

Check each command's exit status. Graph conversion can use roughly 12 GiB
per process for the largest input. The downloads come from the configured
SuiteSparse collection endpoints. The existing converter/reference scripts
are part of the workload definition; do not substitute different graph data.
`PYTHON=/absolute/path/to/python3` overrides the preprocessing interpreter.
Black–Scholes's compressed checked-in input is unpacked automatically by its
run target; `xz` must be installed. Smith–Waterman's sequences and reference
scores are already tracked. AES requires its pinned `tiny-AES-c` submodule.

The current `tests.mk` files specify 34 cases: AES 1, Barnes–Hut 3, BFS 6,
Black–Scholes 1, FFT 4 (two each in `fft/128` and `fft/256`), SGEMM 1,
Jacobi 3, PageRank 7, Smith–Waterman 1, SpGEMM 4, memcpy 1, vector-add 2.
`apps/legacy` is explicitly unsupported and not part of this suite. Logical
`pod-id` values select workload partitions; the simulated physical machine
remains one 16×8 pod.

## Compile a generated case and execute its existing checker

For example, the warm vector-add case is
`$HB/apps/vector_add/tile-x_16__tile-y_8__vector-size_65536__warm-cache_yes`.
From a generated case directory, use:

```sh
# GCC device compilation, with the shared GNU runtime and linker.
gmake -f "$BSG_MACHINE_PATH/Makefile.machine.include" -f Makefile \
  SHELL=/bin/bash '.SHELLFLAGS=-e -o pipefail -c' \
  CC=/usr/bin/clang CXX=/usr/bin/clang++ RISCV_OPT_LEVEL=-O3 \
  main.so main.riscv
/usr/bin/time -p gmake -f "$BSG_MACHINE_PATH/Makefile.machine.include" -f Makefile \
  SHELL=/bin/bash '.SHELLFLAGS=-e -o pipefail -c' \
  CC=/usr/bin/clang CXX=/usr/bin/clang++ RISCV_OPT_LEVEL=-O3 exec.log
```

In the separate LLVM worktree, set `LLVM_PREFIX` to Bladerunner's
`install/llvm22`, then use:

```sh
gmake -f "$BSG_MACHINE_PATH/Makefile.machine.include" -f Makefile \
  -f "$HB/mk/llvm-hammerblade.mk" \
  SHELL=/bin/bash '.SHELLFLAGS=-e -o pipefail -c' \
  CC=/usr/bin/clang CXX=/usr/bin/clang++ RISCV_OPT_LEVEL=-O3 \
  RISCV_LLVM_PATH="$LLVM_PREFIX" RISCV_LLVM_OBJECT_OUTPUT=1 \
  RISCV_LLVM_OPT_FLAGS=-enable-dfa-jump-thread main.so main.riscv
/usr/bin/time -p gmake -f "$BSG_MACHINE_PATH/Makefile.machine.include" -f Makefile \
  -f "$HB/mk/llvm-hammerblade.mk" \
  SHELL=/bin/bash '.SHELLFLAGS=-e -o pipefail -c' \
  CC=/usr/bin/clang CXX=/usr/bin/clang++ RISCV_OPT_LEVEL=-O3 \
  RISCV_LLVM_PATH="$LLVM_PREFIX" RISCV_LLVM_OBJECT_OUTPUT=1 \
  RISCV_LLVM_OPT_FLAGS=-enable-dfa-jump-thread exec.log
```

To execute all generated cases in a **fresh** worktree, the following Bash
loop uses the same flags and runs serially. Run it once with `compiler=gcc`
in the GCC worktree and once with `compiler=llvm` in the LLVM worktree, setting
`HB` accordingly. All input preparation above must have completed first.

```bash
compiler=gcc # or llvm, with HB pointing at the separate LLVM worktree
make_args=(-f "$BSG_MACHINE_PATH/Makefile.machine.include" -f Makefile
  SHELL=/bin/bash '.SHELLFLAGS=-e -o pipefail -c'
  CC=/usr/bin/clang CXX=/usr/bin/clang++ RISCV_OPT_LEVEL=-O3)
if [ "$compiler" = llvm ]; then
  make_args+=(-f "$HB/mk/llvm-hammerblade.mk"
    "RISCV_LLVM_PATH=$LLVM_PREFIX" RISCV_LLVM_OBJECT_OUTPUT=1
    RISCV_LLVM_OPT_FLAGS=-enable-dfa-jump-thread)
fi
for app in aes barnes_hut bfs blackscholes fft/128 fft/256 sgemm jacobi pagerank smithwaterman spgemm memcpy vector_add; do
  for case_dir in "$HB/apps/$app"/*; do
    test -f "$case_dir/parameters.mk" || continue
    gmake -C "$case_dir" "${make_args[@]}" main.so main.riscv || exit 1
    /usr/bin/time -p gmake -C "$case_dir" "${make_args[@]}" exec.log || exit 1
    grep -q 'BSG REGRESSION TEST .*PASSED' "$case_dir/exec.log" || exit 1
  done
done
```

Keep the same compiler options when invoking the run target; Make
does not track command-line changes as dependencies. `RISCV_OPT_LEVEL=-O3`
sets the effective optimization level, including the SDK's appended flags.
Other application arithmetic flags remain unchanged; the SDK adds
`-ffast-math -ffp-contract=off`. LLVM uses its integrated object assembler and
strips unsupported metadata before linking with the existing GNU binutils;
the host harness and common runtime are not a second LLVM implementation.

A successful build is not a successful benchmark. Require exit status zero,
the `BSG REGRESSION TEST PASSED` banner, no failure banner, and complete tile
start/finish markers in `simple_stats.csv`. This invokes each existing checker
and inherits its documented tolerance and coverage; it is not a new oracle.
`profile.log` can replace `exec.log` to collect detailed hardware counters.
Save each run's output before another mode overwrites shared CSV filenames.
Do not use simulator wall time as a compiler speedup metric.
