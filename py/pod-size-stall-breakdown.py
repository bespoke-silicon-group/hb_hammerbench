import numpy as np
import sys
import re
import pandas
from vanilla_parser.stats_parser import CudaStatTag
from itertools import product

order = ['16x16', '16x8', '4x2', '4x4', '8x4', '8x8']
if sys.argv[1] == 'exclude-16x8':
    order = ['16x16', '4x2', '4x4', '8x4', '8x8']

test_dirs = sys.argv[2:]

stall_types = [
    "stall_depend_dram_load",
    "stall_depend_group_load",
    "stall_depend_global_load",
    "stall_depend_idiv",
    "stall_depend_fdiv",
    "stall_depend_local_load",
    "stall_depend_imul",
    "stall_amo_aq",
    "stall_amo_rl",
    "stall_bypass",
    "stall_lr_aq",
    "stall_fence",
    "stall_remote_req",
    "stall_remote_credit",
    "stall_fdiv_busy",
    "stall_idiv_busy",
    "stall_fcsr",
    "stall_barrier",
    "stall_remote_ld_wb",
    "stall_ifetch_wait",
    "stall_remote_flw_wb"
]

r = {}

def stall_breakdown(x,y,data):
    data = data[data['x']==x]
    data = data[data['y']==y]
    start = data[data['tag_type']=='Start']
    end   = data[data['tag_type']=='End']
    stalls_start = np.zeros(len(stall_types),dtype=int)
    stalls_end   = np.zeros(len(stall_types),dtype=int)
    for (idx, stall_type) in enumerate(stall_types):
        stalls_start[idx] = int(start[stall_type])
        stalls_end[idx]   = int(  end[stall_type])
    return stalls_end-stalls_start

def aggregate_stall_breakdown(data):
    stall_breakdowns = []
    # mark the tag type (i.e. begin/end)
    data['tag_type']=data['tag'].apply(
        lambda tag : CudaStatTag(tag).getAction
    )
    num_x = data['x'].nunique()
    num_y = data['y'].nunique()
    for (x,y) in product(range(num_x),range(num_y)):
        stall_breakdowns.append(stall_breakdown(x,y,data))
    return sum(stall_breakdowns)

for test_dir in test_dirs:
    data = pandas.read_csv(test_dir + '/vanilla_stats.csv')
    stalls = aggregate_stall_breakdown(data)
    x = re.search(r'\-x_(\d+)', test_dir).group(1)
    y = re.search(r'\-y_(\d+)', test_dir).group(1)
    r["%sx%s"%(x,y)]={
        stall_type : stalls[idx]
        for (idx,stall_type) in enumerate(stall_types)
    }

fmt = " ".join(
    ["{%s:9}"%stall_type for stall_type in stall_types]
)

for pod_size in order:
    stall_breakdown = r[pod_size]
    print(fmt.format(**stall_breakdown))
