def test_str(nbodies, run):
    return f'TESTS += $(call test-name,{nbodies},0,{run})'

header = """
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[num-iter],[pod-id],[run])
"""

inputs = [64*1024, 1024]
runs = 10

print(header)

for run in range(runs):
    for inp in inputs:
        print(test_str(inp,run))
