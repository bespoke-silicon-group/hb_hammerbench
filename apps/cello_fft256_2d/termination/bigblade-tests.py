def test_str(num_iter, tiles_x, tiles_y, pods_x, pods_y, opt_term, run):
    return f'TESTS += $(call test-name,{num_iter},{tiles_x},{tiles_y},{pods_x},{pods_y},{opt_term},{run})'

header = """
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[num-iter],[tiles-x],[tiles-y],[pods-x],[pods-y],[opt_term],[run])
"""

inputs = [16]

cores = [(16,8,4,2)]

runs = 1

print(header)

for run in range(runs):
    for opt_term in ("no","yes"):
        for inp in inputs:
            for (tiles_x,tiles_y,pods_x,pods_y) in cores:
                print(test_str(inp,tiles_x,tiles_y,pods_x,pods_y,opt_term,run))
