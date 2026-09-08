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
The input generator, kernels, timing markers, and SSE thresholds are unchanged.
