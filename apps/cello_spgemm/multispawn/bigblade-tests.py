def test_str(mtx_a, mtx_b, multispawn, tiles_x, tiles_y, pods_x, pods_y, run):
    return f'TESTS += $(call test-name,{mtx_a},{mtx_b},{tiles_x},{tiles_y},{pods_x},{pods_y},{multispawn},{run})'

header = """
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[mtx-a],[mtx-b],[tiles-x],[tiles-y],[pods-x],[pods-y])
"""

inputs = ['email-Enron', 'roadNet-CA']

cores = [(16,8,4,2)]
runs = 10

print(header)

for run in range(runs):
    for multispawn in ("yes", "no"):
        for inp in inputs:
            for (tiles_x,tiles_y,pods_x,pods_y) in cores:
                print(test_str(inp,inp,multispawn,tiles_x,tiles_y,pods_x,pods_y,run))

