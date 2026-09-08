#!/usr/bin/env python3
"""Generate small exact CSR A^2 fixtures with an independent sparse reference.

Uses integer dictionary accumulation, then verifies against dense triple loops.
No SciPy dependency or kernel arithmetic model. Structural zeros are retained.
"""
import argparse
from pathlib import Path

ROWS = [
    {1: 2, 2: -3, 3: 4},
    {0: 5, 3: 2, 6: 1},
    {0: 2, 2: -1, 5: 3},
    {1: -2, 3: 1, 7: 4},
    {},
    {0: 1, 4: 2},
    {2: 2},
    {4: -1, 6: 3},
]


def sparse_square(rows):
    result = []
    for row in rows:
        sums = {}
        for k, a in sorted(row.items()):
            for j, b in sorted(rows[k].items()):
                sums[j] = sums.get(j, 0) + a * b
        result.append(sums)
    return result


def csr(rows):
    offsets, columns, values = [0], [], []
    for row in rows:
        for j, v in sorted(row.items()):
            columns.append(j)
            values.append(v)
        offsets.append(len(columns))
    return offsets, columns, values


def generate(out):
    out.mkdir(parents=True, exist_ok=True)
    cancellation = [dict(row) for row in ROWS]
    cancellation[0][2] = -5  # C[0,0] = 2*5 - 5*2 = 0; retain its structural entry.
    for name, rows in (("weighted-regression", ROWS),
                       ("unit-regression", [{j: 1 for j in row} for row in ROWS]),
                       ("cancellation-regression", cancellation)):
        result = sparse_square(rows)
        n = len(rows)
        # Independent dense traversal checks both values and structural output keys.
        for i in range(n):
            for j in range(n):
                expected = sum(rows[i].get(k, 0) * rows[k].get(j, 0) for k in range(n))
                structural = any(k in rows[i] and j in rows[k] for k in range(n))
                assert (j in result[i]) == structural
                assert result[i].get(j, 0) == expected
        source = csr(rows)
        target = csr(result)
        assert max(map(abs, source[2] + target[2])) < 2**24
        for suffix, values in zip(("row_offset", "col_idx", "nnz",
                                   "output_row_offset", "output_col_idx", "output_nnz"),
                                  source + target):
            (out / f"{name}.{suffix}.txt").write_text("\n".join(map(str, values)) + "\n")
        (out / f"config.{name}.mk").write_text(
            f"VERTEX={n}\nEDGE={len(source[1])}\nOUTPUT_EDGE={len(target[1])}\n"
            "POD_DIM_X=1\nPOD_DIM_Y=1\n")
        print(f"{name}: V={n}, E={len(source[1])}, OUTPUT_E={len(target[1])}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("output_dir", type=Path)
    generate(parser.parse_args().output_dir)
