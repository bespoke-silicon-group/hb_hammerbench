import numpy as np
import sys
import re
import pandas
import argparse
from vanilla_parser.stats_parser import CudaStatTag
from itertools import product

parser = argparse.ArgumentParser()
parser.add_argument("test_dirs", nargs="+")
parser.add_argument("--spgemm", action="store_true")
parser.add_argument("--exclude-16x8", action="store_true")
arguments = parser.parse_args()

order = ['16x16', '16x8', '4x2', '4x4', '8x4', '8x8']
if arguments.exclude_16x8:
    order = ['16x16', '4x2', '4x4', '8x4', '8x8']

test_dirs = arguments.test_dirs

stall_types = [
    "instr_total",
    "bubble_branch_miss",
    "bubble_jalr_miss",
    "bubble_icache_miss",
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
total_cycles={ s : 0 for s in order }
remote_lds = { s : 0 for s in order }

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

def cycles(x,y,data):
    data = data[data['x']==x]
    data = data[data['y']==y]
    start = data[data['tag_type']=='Start']
    end   = data[data['tag_type']=='End']
    return int(end['cycle'].sum()-start['cycle'].sum())

def remote_loads(x,y,data):
    data = data[data['x']==x]
    data = data[data['y']==y]
    start = data[data['tag_type']=='Start']
    end   = data[data['tag_type']=='End']
    return int(end['instr_remote_ld_dram'].sum()-start['instr_remote_ld_dram'].sum())

def aggregate_stall_breakdown(data):
    stall_breakdowns = []
    # mark the tag type (i.e. begin/end)
    data['tag_type']=data['tag'].apply(
        lambda tag : ('Start' if CudaStatTag(tag).isKernelStart else
                      ('End' if CudaStatTag(tag).isKernelEnd else
                       'Other'))
    )
    num_x = data['x'].nunique()
    num_y = data['y'].nunique()
    for (x,y) in product(range(num_x),range(num_y)):
        sb = stall_breakdown(x,y,data)
        cyc = cycles(x,y,data)
        total_cycles['%dx%d'%(num_x,num_y)] += cyc
        remote_lds['%dx%d'%(num_x,num_y)] += remote_loads(x,y,data)
        print("sum(breakdown)={}, cycles={}, equal={}".format(
            sum(sb), cyc, sum(sb)==cyc
        ))
        stall_breakdowns.append(sb)
    return sum(stall_breakdowns)

for test_dir in test_dirs:
    data = pandas.read_csv(test_dir + '/vanilla_stats.csv')
    #print('\n'.join(data.columns.to_list()))
    stalls = aggregate_stall_breakdown(data)
    if arguments.spgemm:
        x = re.search(r'(\d+)_tiles-x', test_dir).group(1)
        y = re.search(r'(\d+)_tiles-y', test_dir).group(1)
    else:
        x = re.search(r'\-x_(\d+)', test_dir).group(1)
        y = re.search(r'\-y_(\d+)', test_dir).group(1)
    r["%sx%s"%(x,y)]={
        stall_type : stalls[idx]
        for (idx,stall_type) in enumerate(stall_types)
    }

fmt = " ".join(
    ["{%s}"%stall_type for stall_type in stall_types]
)

hdr = " ".join(
    ["{}" for stall_type in stall_types]
)

# print(hdr.format(*stall_types))
# for pod_size in order:
#     stall_breakdown = r[pod_size]
#     print(fmt.format(**stall_breakdown))

fmt = "{:25} " + " ".join(["{:10}"]*len(order))
hdr = "{:25} " + " ".join(["{:>10}"]*len(order))
print(hdr.format("TYPE", *order))
for stall_type in stall_types:
    sbd = [0]*len(order)
    for idx,pod_size in enumerate(order):
        stall_breakdown = r[pod_size]
        sbd[idx] = stall_breakdown[stall_type]
    #print(sbd)
    print(fmt.format(stall_type, *sbd))

breakdown_total = np.array([sum([stall_breakdown[stall_type] for stall_type in stall_types]) for stall_breakdown in [r[s] for s in order]])
cycles_total    = np.array([total_cycles[s] for s in order])
remote_loads_total    = np.array([remote_lds[s] for s in order])

print(fmt.format("TOTAL BREAKDOWN", *breakdown_total))
print(fmt.format("TOTAL CYCLES", *cycles_total))
print(fmt.format("DIFF", *(cycles_total-breakdown_total)))
print(fmt.format("REMOTE LOADS", *remote_loads_total))
    #print(("{}" + " ".join("{}"*len(order))).format(pod_size, *sbd))
