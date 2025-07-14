def test_str(num_iter, run):
    return f'TESTS += $(call test-name,{num_iter},{run})'

header = """
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[num-iter],[run])
"""

inputs = [16]
runs = 10

print(header)

for inp in inputs:
    for run in range(runs):
        print(test_str(inp,run))
