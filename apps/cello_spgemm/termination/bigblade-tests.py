def test_str(mtx_a, mtx_b, opt_term, tiles_x, tiles_y, pods_x, pods_y, run):
    return f'TESTS += $(call test-name,{mtx_a},{mtx_b},{tiles_x},{tiles_y},{pods_x},{pods_y},{opt_term},{run})'

header = """
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[mtx-a],[mtx-b],[tiles-x],[tiles-y],[pods-x],[pods-y])
"""

inputs = ['u12k16']

cores = [(16,8,4,2)]
runs = 1

print(header)

for run in range(runs):
    for opt_term in ("yes", "no"):
        for inp in inputs:
            for (tiles_x,tiles_y,pods_x,pods_y) in cores:
                print(test_str(inp,inp,opt_term,tiles_x,tiles_y,pods_x,pods_y,run))

