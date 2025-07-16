import argparse
import itertools

def test_str(input_str, opt_memcpy, opt_restrict_ws, opt_lock, run):
    return f'TESTS += $(call test-name,{input_str},{opt_memcpy},{opt_restrict_ws},{opt_lock},{run})'

header = """
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[input],[opt-memcpy],[opt-restrict-ws],[opt-lock],[run])
"""

inputs = ['email-Enron', 'wiki-Vote']

parser = argparse.ArgumentParser()
parser.add_argument("--runs", type=int, default=1, help="how many times to run each setting")
args = parser.parse_args()

runs = args.runs
opts = [
    # (opt_memcpy, opt_restrict_ws, opt_lock)
    ("no" ,"no" ,"no" ),
    ("yes","no" ,"no" ),
    ("yes","yes","no" ),
    ("yes","yes","yes"),
]

print(header)

for inp, run, (opt_memcpy, opt_restrict_ws, opt_lock) in itertools.product(inputs, range(runs), opts):
    print(test_str(inp, opt_memcpy, opt_restrict_ws, opt_lock, run))
