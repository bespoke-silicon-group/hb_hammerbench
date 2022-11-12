import sys
import re
test_dirs = sys.argv[1:]

r = {}

for test_dir in test_dirs:
    stats_log = test_dir + '/stats/manycore_stats.log'
    with open(stats_log,'r') as stats_file:
        _ = stats_file.readline()
        _ = stats_file.readline()
        _ = stats_file.readline()
        l = stats_file.readline()
        l = [c for c in l.split(' ') if c]
        instrs = l[1]
        cycles = l[6]
        x = re.search(r'x_(\d+)', test_dir).group(1)
        y = re.search(r'y_(\d+)', test_dir).group(1)
        r['%sx%s'%(x,y)] = (instrs,cycles)

order = ['16x16', '16x8', '4x2', '4x4', '8x4', '8x8']

for pod_size in order:
    instr,cycles = r[pod_size]
    print("{} {}".format(instr,cycles))
