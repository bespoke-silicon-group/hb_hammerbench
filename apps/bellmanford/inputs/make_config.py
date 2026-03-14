import argparse

def parse_mtx_header(path):
    """
    Reads the Matrix Market file and extracts:
    - number of vertices
    - number of edges
    Assumes coordinate format.
    """
    with open(path, "r") as f:
        for line in f:
            if line.startswith("%"):
                continue
            parts = line.strip().split()
            if len(parts) == 3:
                # First non-comment line: rows cols nnz
                rows, cols, nnz = map(int, parts)
                return rows, nnz

    raise ValueError("Invalid MTX file: could not find header line.")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--graph", required=True, help="Input .mtx file")
    parser.add_argument("--out", required=True, help="Output config file path")
    parser.add_argument("--root", type=int, default=0)
    parser.add_argument("--numpods", type=int, default=1)
    parser.add_argument("--tgx", type=int, default=4)
    parser.add_argument("--tgy", type=int, default=4)

    args = parser.parse_args()

    vertices, edges = parse_mtx_header(args.graph)

    with open(args.out, "w") as f:
        f.write(f"VERTEX={vertices}\n")
        f.write(f"EDGE={edges}\n")
        f.write(f"ROOT={args.root}\n")
        f.write(f"NUMPODS={args.numpods}\n")
        f.write(f"TILE_GROUP_DIM_X={args.tgx}\n")
        f.write(f"TILE_GROUP_DIM_Y={args.tgy}\n")

    print(f"Wrote config to {args.out}")


if __name__ == "__main__":
    main()