import argparse
import random

parser = argparse.ArgumentParser()
parser.add_argument('graph_name')
parser.add_argument('num_vertices')
parser.add_argument('num_edges')
parser.add_argument('weight_low')
parser.add_argument('weight_high')
parser.add_argument('allow_self_edges')

args = parser.parse_args()

def generate_graph(num_vertices, num_edges, w_low, w_high, allow_self):
    # error check parameters
    max_edges = num_vertices * num_vertices
    max_edges_no_self = num_vertices * (num_vertices - 1)

    if num_edges > max_edges or ((not allow_self) and num_edges > max_edges_no_self):
        return None

    # adjacency matrix initialized to 0
    matrix = [[0.0 for _ in range(num_vertices)] for _ in range(num_vertices)]

    edges_added = 0
    while edges_added < num_edges:
        x = random.randint(0, num_vertices - 1)
        y = random.randint(0, num_vertices - 1)

        if not allow_self and x == y:
            continue

        if matrix[x][y] == 0:
            matrix[x][y] = random.uniform(w_low, w_high)
            edges_added += 1

    return matrix


# generate the random graph
matrix = generate_graph(
    int(args.num_vertices),
    int(args.num_edges),
    float(args.weight_low),
    float(args.weight_high),
    bool(args.allow_self_edges == "True")
)

if matrix is None:
    print("Invalid graph parameters!")
    exit(1)

# Write Matrix Market (.mtx) manually
# Format:
# %%MatrixMarket matrix coordinate real general
# nrows ncols nnz
# i j value

out_path = args.graph_name

with open(out_path, "w") as f:
    f.write("%%MatrixMarket matrix coordinate real general\n")
    f.write("% Generated without external libraries\n")

    n = len(matrix)
    nnz = sum(1 for i in range(n) for j in range(n) if matrix[i][j] != 0)

    f.write(f"{n} {n} {nnz}\n")

    for i in range(n):
        for j in range(n):
            if matrix[i][j] != 0:
                # Matrix Market is 1-indexed
                f.write(f"{i+1} {j+1} {matrix[i][j]}\n")