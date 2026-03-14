import argparse
import numpy as np
import networkx as nx
from scipy.io import mmwrite

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
  if ((num_edges > (num_vertices * num_vertices)) or 
      ((not allow_self) and (num_edges > (num_vertices * (num_vertices -1))))):
    print("test")
    print(num_edges)
    print(num_vertices)
    return None
  
  # make an edgeless graph with the required number of vertices
  matrix = np.zeros((num_vertices, num_vertices))

  # fill it with edges!
  rng = np.random.default_rng()
  for i in range(num_edges):
    # somewhat inefficiently figure out random edges
    while(True):
      x = np.random.randint(low=0, high=num_vertices)
      y = np.random.randint(low=0, high=num_vertices)

      # if we don't want self-edges, reroll
      while ((not allow_self) and (y == x)):
        y = np.random.randint(low=0, high=num_vertices)
      
      if (matrix[x][y] == 0):
        matrix[x][y] = rng.uniform(low=w_low, high=w_high)
        break
  
  return matrix

# generate the random graph
matrix = generate_graph(int(args.num_vertices), 
                       int(args.num_edges), 
                       float(args.weight_low), 
                       float(args.weight_high), 
                       bool(args.allow_self_edges == "True"))

# error check
if (matrix is None):
  print("Invalid graph parameters!")
  exit(1)

# DEBUG
# print(matrix)

# write it to the file
graph = nx.from_numpy_matrix(matrix, create_using=nx.DiGraph())
sci_array = nx.to_scipy_sparse_matrix(graph)
mmwrite(args.graph_name, sci_array)
