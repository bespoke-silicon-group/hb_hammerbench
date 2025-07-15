def test_str(nbodies, run):
    return f'TESTS += $(call test-name,{nbodies},{run})'

header = """
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[num-iter],[run])
"""

inputs = [64*1024]
runs = 10

print(header)

for inp in inputs:
    for run in range(runs):
        print(test_str(inp,run))
