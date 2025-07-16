def test_str(num_iter, run):
    return f'TESTS += $(call test-name,{num_iter},{run})'

header = """
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[num-iter],[run])
"""

inputs = [32*2*1024//128, 1024//128]
runs = 10

print(header)

for run in range(runs):
    for inp in inputs:
        print(test_str(inp,run))
