# Black–Scholes checker contract

The per-pod FP32 sum of call/put squared errors still uses the original option
order and **0.01** limit. The pricing implementations, input distribution,
input reading, device code, warmup, and timing markers are unchanged. This
checker correction does not extend or certify the approximation's input domain.

The host now rejects NaN/infinity in either actual or reference call/put values
and rejects non-finite accumulated SSE. Diagnostics identify the first affected
field, pod, option index, and actual/reference values, followed by an invalid
count. The original checker could accept a NaN-poisoned SSE.

The shared host-only `../common/host_sse.hpp` check classifies binary32 through
`memcpy` and a volatile integer exponent mask. Its regression tests the original
PageRank/Black–Scholes thresholds, finite errors, NaN and both infinities on
either/both sides, and arithmetic overflow. Run
`../common/test_host_sse.cpp` with the effective host flags; an additional
`-O3 -ffast-math` build stresses finite rejection. This stress check does not
claim reference arithmetic is unchanged under fast-math.
