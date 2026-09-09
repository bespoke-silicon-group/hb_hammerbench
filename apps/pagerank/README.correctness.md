# PageRank checker contract

This remains the shipped single pull update: `(1-damp)/EDGE`, uniform initial
contributions, and the existing dangling-vertex handling. It does not certify
general normalized PageRank or convergence. The two selected-range FP32 SSE
limits remain **0.001**, one each for contribution and rank, in the original
per-field vertex order.

The host now rejects every NaN/infinity in actual or reference outputs, including
paired infinities that the original checker accepted. Non-finite differences
or accumulated SSE also fail. Diagnostics identify the first affected field,
pod, vertex index, and actual/reference values, then report per-field invalid
counts. A failed check still reaches device finalization.

The host-only `../common/host_sse.hpp` class uses a copied binary32 integer
representation with a volatile integer boundary, so finite classification is
not folded away by finite-math assumptions. Its shared regression covers both
original application thresholds, NaN and both infinities on either/both sides,
finite errors, and arithmetic overflow:

```sh
c++ -std=c++11 ../common/test_host_sse.cpp -o /tmp/host-sse-test
/tmp/host-sse-test
c++ -std=c++11 -O3 -ffast-math ../common/test_host_sse.cpp -o /tmp/host-sse-fast-test
/tmp/host-sse-fast-test
```

Fast-math stress validation here concerns rejection of non-finite results,
not equivalence of numerical reference computations under changed compiler flags.
The input generator, timing markers, and SSE thresholds are unchanged.

## Constant initialization and zero-indegree check

The selected CUDA-lite startup does not invoke C++ `.init_array` functions.
Initializing `beta_score` from writable `damp` previously generated such a
function, leaving the device offset zero. Initialize both ordinary data globals
from the `constexpr initial_damp` value instead. This removes the initializer
while retaining their DMEM placement under the local-data link; making the
globals themselves `constexpr` instead puts their constants in DRAM with the
current linker.

The host additionally checks vertices with no incoming edges. Their rank must
equal `beta_score` exactly, and their contribution is one FP32 multiplication
by the degree reciprocal. This check runs after device execution and readback,
outside kernel timing. It catches the missing offset even when the aggregate
SSE accepts it. A graph/range without such vertices reports `checked=0` and
still uses the existing SSE checks; this is not a general new accuracy bound.

```sh
c++ -std=c++11 -O2 test_verification.cpp -o /tmp/pagerank-verification
/tmp/pagerank-verification
```

The regression rejects missing rank/contribution offsets, a one-ULP rank error,
NaN and infinities. On wiki-Vote partition 36 at 128 tiles, the old fenced kernel
fails all 97 zero-indegree checks; constant initialization passes all 97.
Two 32-byte source padding arrays replace the removed initializer's instruction
and DRAM-data footprint in the GCC 9.2 build. This preserves graph allocation
addresses and instruction-cache loading time; without padding, those changes
shifted memory contention and DRAM refresh relative to kernel execution.
With padding, measured target cycles remain **14,497→14,497** with the same
strengthened host, inputs and default 16×8 profile. Per-tile timed counters,
cache-window counters and DRAM-window command counts also match. Startup
cumulative counters differ. Recheck image sizes if the compiler or flags change.
The formula remains `(1-damp)/EDGE`, and no general C++ constructor support
is added to the runtime.
