def test_str(num_iter, tiles_x, tiles_y, pods_x, pods_y, multispawn, run):
    return f'TESTS += $(call test-name,{num_iter},{tiles_x},{tiles_y},{pods_x},{pods_y},{multispawn},{run})'

header = """
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[num-iter],[tiles-x],[tiles-y],[pods-x],[pods-y],[multispawn],[run])
"""

inputs = [8*1024]

cores = [
    (16,8,4,2)
]

runs = 10

print(header)

for run in range(runs):
    for multispawn in ("yes","no"):
        for inp in inputs:
            for (tiles_x,tiles_y,pods_x,pods_y) in cores:
                print(test_str(inp, tiles_x, tiles_y, pods_x, pods_y, multispawn, run))
