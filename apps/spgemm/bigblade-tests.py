def test_str(graph, run):
    return f'TESTS += $(call test-name,{graph},0,{run})'

header = """
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[graph],[pod-id],[run])
"""

inputs = ['wiki-Vote','roadNet-CA']
runs = 10

print(header)

for run in range(runs):
    for inp in inputs:
        print(test_str(inp,run))
