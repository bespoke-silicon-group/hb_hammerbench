def test_str(num_iter, tiles_x, tiles_y, pods_x, pods_y, unrestricted, run):
    return f'TESTS += $(call test-name,{num_iter},{tiles_x},{tiles_y},{pods_x},{pods_y},{unrestricted},{run})'

header = """
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[num-iter],[tiles-x],[tiles-y],[pods-x],[pods-y],[unrestricted],[run])
"""

inputs = [64*1024]

cores = [
    (16,8,4,2)
]

runs = 10

print(header)

for run in range(runs):
    for unrestricted in ("no","yes"):
        for inp in inputs:
            for (tiles_x,tiles_y,pods_x,pods_y) in cores:
                print(test_str(inp, tiles_x, tiles_y, pods_x, pods_y, unrestricted, run))
