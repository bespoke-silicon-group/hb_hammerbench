# SpGEMM correctness contract

The kernel computes weighted FP32 CSR multiplication. The existing host forms
A² by extracting A rows and B columns from the same input matrix. Inputs must
have sorted, unique column IDs in each row, valid row offsets/indices, and finite
values with finite arithmetic intermediates. Every structural output entry is
retained, including exact zeros produced by cancellation; the supplied output
CSR must use that convention. Pool capacity and output capacity must be adequate.

The expert `inputs/convert_mtx.py` is unchanged: it converts nonzeros to unit
weights and generates adjacency-path counts. Those original inputs remain the
unit-weight benchmark baseline. Previously, a first appearance of an output
column copied B alone while duplicate-column merges used A*B+C. Weighted
failures from that arithmetic are software failures, not evidence of a hardware
fault. First appearances now multiply by A in both insertion paths. Duplicate
merges retain the original fused multiply-add and order; inputs/expected values
must respect that FP32 arithmetic when products are not exactly representable.
The host's exact offset, ID, and value comparisons remain unchanged.

Generate independent small correctness inputs:

```sh
python3 inputs/generate_weighted_regression.py /absolute/fixture-directory
```

This creates weighted, unit, and cancellation variants, each with 8 rows,
17 input entries, and 30 structural output entries. The weighted matrices use
small signed integers, so products and sums are exactly representable in FP32.
They exercise first insertions, both sides of sorted merges, duplicate-column
accumulation, empty rows, and a retained cancellation zero. Integer dictionary
accumulation supplies the sparse reference; independent dense triple loops
cross-check its values and structure. No kernel arithmetic model or external
sparse library generates the reference.

The generated `config.<variant>.mk` files belong beside this app's existing
config files when using the normal `graph_<variant>__pod-id_0` generation flow.
The six generated text files can go in `inputs`, or be supplied through the
six host command-line paths after `main.riscv`. The original `tests.mk` is
unchanged; select the regression explicitly. A 2×2 physical model requires
`tile-x=2 tile-y=2 TREE_LEVELS=3`, with the matching `BSG_MACHINE_PATH`.
`TREE_LEVELS=1+log2(tile-x*tile-y)` is required for a power-of-two tile count.
This arithmetic correction does not change the original fixed tree-depth
default, pool exhaustion handling, input validation, or output-overrun risks.
