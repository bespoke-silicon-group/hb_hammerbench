# Barnes–Hut correctness contract

The host reference and kernel share theta=0.5 (`itolsq=4`), softening squared
`0.05*0.05`, and velocity increment `0.25*(new_acc-old_acc)`. The existing
per-pod FP32 sum of squared acceleration and velocity errors is initialized to
zero and accepted at **SSE <= 0.01**, provided it is finite. A NaN or infinity
cannot establish agreement. This retains the expert finite-error threshold;
it does not establish a tighter componentwise bound.

Child EVAs tag leaves in bit zero. Clear that bit before casting to `HBBody*`,
dereferencing, or comparing against the current body's untagged address.
The original tagged comparison could never recognize self; passing a hardware
run did not validate that pointer contract. The host now finishes the device
on both verification outcomes, allowing simulator finalizers to flush profiles.

The input generator, force arithmetic, ownership, timing markers, and absence
of a warmup remain unchanged. The fixed traversal stack is still bounded only
by its allocation; this correction does not establish arbitrary-tree stack
safety or multipod correctness.

Run the host-only boundary/tag regression from this directory without
`-ffast-math` (the normal host build does not enable it):

```sh
c++ -std=c++11 -Wall -Wextra tests/regression.cpp -o /tmp/barnes-hut-regression
/tmp/barnes-hut-regression
```

Device validation must still use a compatible physical model. A useful small
case is `nbodies_256__pod-id_0` with `NUMPODS=1`, explicit `tile-x`, `tile-y`,
and `BSG_MACHINE_PATH`. The original checker reports SSE about 0.010248 for the
seeded 256-body input; fixing only the host reference isolates that failure
from device pointer decoding. Fault-injecting NaN into a returned acceleration
provides a separate rejection check; keep that injection outside benchmark
sources and measured device kernels.
