import sys
import core_utilization as co

# input arguments
graph         = sys.argv[1]
numpods       = int(sys.argv[2])


max_runtime = 0
max_pid = 0
for pid in range(numpods):
  # parse
  filename = "graph_{}__pod-id_{}/vanilla_stats.csv".format(graph, pid)
  stats = co.parse_vanilla_stat(filename)
  if stats is None:
    continue
  curr_runtime = stats["runtime"]
  if max_runtime < curr_runtime:
    max_runtime = curr_runtime
    max_pid = pid
  print("pid={}, runtime={}".format(pid, curr_runtime))
print("max_pid={}, runtime={}".format(max_pid, max_runtime))
