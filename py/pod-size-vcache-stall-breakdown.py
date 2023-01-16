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
    "stall_idle",    
    "stall_miss",
    "stall_rsp",
]

r = {}

def filter_min_time(data):
    min_time = data['time'].min()
    return data[data['time']==min_time]

def filter_max_time(data):
    max_time = data['time'].max()
    return data[data['time']==max_time]
    
def stall_breakdown(vc,data):
    data = data[data['vcache']==vc]
    # unambiguate using the min/max time
    start = data[data['tag_type']=='Start']
    start = filter_min_time(data)
    end   = data[data['tag_type']=='End']
    end   = filter_max_time(data)
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
        lambda tag : ('Start' if CudaStatTag(tag).isKernelStart else
                      ('End' if CudaStatTag(tag).isKernelEnd else
                       'Other'))
    )
    vcaches = data['vcache'].unique()
    for vc in vcaches:
        stall_breakdowns.append(stall_breakdown(vc, data))
    return sum(stall_breakdowns)

for test_dir in test_dirs:
    data = pandas.read_csv(test_dir + '/vcache_stats.csv')
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

for pod_size in order:
    stall_breakdown = r[pod_size]
    print(fmt.format(**stall_breakdown))
