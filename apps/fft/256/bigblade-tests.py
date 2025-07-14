def test_str(num_iter, run):
    return f'TESTS += $(call test-name,{num_iter},no,{run})'

header = """
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[num-iter],[warm-cache],[run])
"""

inputs = [16]
runs = 10

print(header)

for run in range(runs):
    for inp in inputs:
        print(test_str(inp,run))
