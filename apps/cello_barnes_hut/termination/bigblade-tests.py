def test_str(nbodies, tiles_x, tiles_y, pods_x, pods_y, opt_term, run):
    return f'TESTS += $(call test-name,{nbodies},{tiles_x},{tiles_y},{pods_x},{pods_y},{opt_term},{run})'

header = """
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[nbodies],[tiles-x],[tiles-y],[pods-x],[pods-y],[opt_term],[run])
"""

inputs = [64*1024]

cores = [(16,8,4,2)]

runs = 1

print(header)

for run in range(runs):
    for opt_term in ("yes","no"):
        for inp in inputs:
            for (tiles_x,tiles_y,pods_x,pods_y) in cores:
                print(test_str(inp, tiles_x, tiles_y, pods_x, pods_y, opt_term, run))
