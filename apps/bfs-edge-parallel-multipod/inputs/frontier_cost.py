import sys

graph_name = sys.argv[1]

# load distance data;
distances = []
with open("{}.distance.txt".format(graph_name), "r") as f0:
  for line in f0:
    dist = int(line.strip())
    distances.append(dist)


max_distance = max(distances)
for i in range(max_distance+1):
  count = distances.count(i)
  # for 8x8, 16x8 pods
  overhead = (count*2)
  print("d={}, frontier_size={}, overhead={}".format(i, count, overhead))
