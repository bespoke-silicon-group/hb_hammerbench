FFT128m enables the host-only `HB_FFT_STRONG_VERIFY` policy. The stronger
fixtures exposed a device defect in the shared `../128/fft128.hpp`:
`twiddle_scaling` wrote lane 1's imaginary product into lane 0's temporary.
The separate correction in commit `1782d2c` writes `res1_im_temp`, preserving
the butterfly algorithm, host twiddle generation, launch geometry, markers
and warmup. Other FFT variants do not enable the stronger host policy, since
their geometry/synchronization constraints require separate execution evidence.

With no extra argument (or `expert`), the original cos(i*pi/8) input and
distance < 15 historical checker remain, labelled `historical-expert`.
The overall result additionally requires the stronger check. Pass `impulse`,
`constant`, or `random` after the device ELF to run the named fixture:

- Impulse at index 37: 1-0.5i, all other values zero.
- Constant: 0.25-0.125i.
- Random: fixed seed-1 unsigned LCG, real/imaginary samples in [-1,1].

Each iteration uses the same selected input. The independent reference is a
general scalar FP64 radix-2 FFT of the actual FP32 input, using host double
trigonometry and no device tables, generated butterflies or P-by-P factoring.
`test_verification.c` checks it against an O(n^2) direct DFT for all fixtures
and analytic transforms for impulse/constant.

Every output component and reference component must be finite. Per-bin complex
error must be <= `5e-4 + 2e-5*abs(reference)`. This explicit fixture policy
allows FP32 butterfly/twiddle rounding while being much stricter than 15;
it is not a bound for arbitrary amplitudes or lengths. Diagnostics include
maximum complex absolute/relative error (relative denominator floor 1e-12)
and error divided by the per-bin limit. IEEE bit classification includes a
volatile integer boundary so fast-math cannot fold it away.

```
clang -std=c99 -O3 -ffast-math apps/fft/common/test_verification.c -lm -o /tmp/fft-check
/tmp/fft-check
```

Tests inject NaN, both infinities and an incorrect finite imaginary component.
