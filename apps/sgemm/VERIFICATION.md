The default `expert` fixture retains A[i]=i%7, B[i]=i%3, the original
FP32 `host_mm`, and the historical per-pod float SSE < 0.01 criterion.
That result is labelled `historical-expert`; it can accept NaN and is not
the full acceptance decision. Kernel code, launch, markers and warmup are unchanged.

Pass `signed` after the device ELF to select seed-1 deterministic signed
non-integer values in [-1,1]. Values come from the shared unsigned LCG;
the two operands consume alternating samples, including across batches.

Both fixtures additionally require every output/reference to be finite and
each FP32 output to match a scalar FP64 dot product within
`1e-6 + 4*gamma_n*sum(abs(a[k]*b[k]))`, where `gamma_n=n*u/(1-n*u)`,
`u=2^-24`, and `n*u<1`. The sum-of-products scale tolerates cancellation
without hiding a large outlier behind an aggregate SSE. This policy allows
FP32 FMA accumulation error; it is not a claim of accuracy on arbitrary
inputs. Inputs must be finite and shapes supported by the existing kernel.
Diagnostics report non-finite count, maximum absolute error, relative error
with denominator floor 1e-12, and maximum error divided by its acceptance limit.

The host checker copies IEEE-754 bits with `memcpy`, then passes them through
a volatile integer observation before testing the exponent. Apple Clang 21
at `-O3 -ffast-math` erased the plain copy-and-mask check; the volatile integer
boundary preserved NaN/Inf rejection in the tested configuration. Verify the
compiled checker when changing compiler or flags:

```
clang++ -std=c++11 -O3 -ffast-math apps/sgemm/test_verification.cpp -o /tmp/sgemm-check
/tmp/sgemm-check
```

The test checks the FP64 reference against a separately traversed long-double
reference, then injects NaN, both infinities and an incorrect finite result.
