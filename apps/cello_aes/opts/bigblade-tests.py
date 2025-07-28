import argparse
import itertools

def test_str(input_str, tiles_x, tiles_y, pods_x, pods_y, opt_memcpy, opt_restrict_ws, opt_lock, opt_rng, opt_icache, run):
    return f'TESTS += $(call test-name,{input_str},{tiles_x},{tiles_y},{pods_x},{pods_y},{opt_memcpy},{opt_restrict_ws},{opt_lock},{opt_rng},{opt_icache},{run})'

header = """
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[num-iter],[tiles-x],[tiles-y],[pods-x],[pods-y],[opt-memcpy],[opt-restrict-ws],[opt-lock],[opt-rng],[opt-icache],[run])
"""

inputs = [8*1024]

parser = argparse.ArgumentParser()
parser.add_argument("--runs", type=int, default=1, help="how many times to run each setting")
args = parser.parse_args()

cores = [(16,8,4,2)]

runs = args.runs
opts = [
    # (opt_memcpy, opt_restrict_ws, opt_lock, opt_rng, opt_icache)
    # in order of likelihood to be good
    ("no", "no", "yes", "no", "no"),    
    ("no", "yes", "yes", "no", "no"),
    ("no", "yes", "no", "no", "no"),
    ("no", "yes", "no", "yes", "no"),
    ("yes", "yes", "no", "yes", "no"),
    ("yes", "yes", "no", "yes", "yes"),    
]

print(header)

for inp, run, (tiles_x, tiles_y, pods_x, pods_y), (opt_memcpy, opt_restrict_ws, opt_lock, opt_rng, opt_icache) in itertools.product(inputs, range(runs), cores, opts):
    print(test_str(inp, tiles_x, tiles_y, pods_x, pods_y, opt_memcpy, opt_restrict_ws, opt_lock, opt_rng, opt_icache, run))
