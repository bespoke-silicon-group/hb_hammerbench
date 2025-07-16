def test_str(num_seq, run):
    return f'TESTS += $(call test-name,{num_seq},{run})'

header = """
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[num-iter],[run])
"""

inputs = [1024*1024, 8*1024]
runs = 10

print(header)

for run in range(runs):
    for inp in inputs:
        print(test_str(inp,run))
