import sys
import core_utilization as co

# input arguments
graph    = sys.argv[1]
niter    = int(sys.argv[2])
numpods  = int(sys.argv[3])


total_runtime = 0
for n in range(niter):
  max_runtime = 0
  max_pid = 0
  for pid in range(numpods):
    # parse
    filename = "graph_{}__niter_{}__pod-id_{}/vanilla_stats.csv".format(graph, n, pid)
    stats = co.parse_vanilla_stat(filename)
    if stats is None:
      continue
    curr_runtime = stats["runtime"]
    if max_runtime < curr_runtime:
      max_runtime = curr_runtime
      max_pid = pid
      #print("pid={}, runtime={}".format(pid, stats["runtime"]))
  print("iter={}, max_pid={}, runtime={}".format(n, max_pid, max_runtime))
  total_runtime += max_runtime

print("total_runtime: {}, adjusted={}".format(total_runtime, total_runtime-(niter*512)))
