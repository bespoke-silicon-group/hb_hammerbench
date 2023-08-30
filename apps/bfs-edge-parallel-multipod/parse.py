import sys
import core_utilization as co

# input arguments
graph    = sys.argv[1]
root     = sys.argv[2]
niter    = sys.argv[3]
numpods  = int(sys.argv[4])


max_runtime = 0
max_pid = 0
for pid in range(numpods):
  # parse
  filename = "graph_{}__root_{}__niter_{}__pod-id_{}/vanilla_stats.csv".format(graph, root, niter, pid)
  stats = co.parse_vanilla_stat(filename)
  curr_runtime = stats["runtime"]
  if max_runtime < curr_runtime:
    max_runtime = curr_runtime
    max_pid = pid
  print("pid={}, runtime={}".format(pid, stats["runtime"]))

print("max_pid={}, runtime={}".format(max_pid, max_runtime))
