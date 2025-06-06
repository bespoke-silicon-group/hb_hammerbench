def test_str(nbodies, tiles_x, tiles_y, pods_x, pods_y, grain_scale):
    return f'TESTS += $(call test-name,{nbodies},{tiles_x},{tiles_y},{pods_x},{pods_y},{grain_scale})'

header = """
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[nbodies],[tiles-x],[tiles-y],[pods-x],[pods-y],[grain])
"""

inputs = [64*1024]

grains = [1,2,3,4,5,6,7,8]
cores = [(16,8,4,2)]

print(header)

for grain in grains:
    for inp in inputs:
        for (tiles_x,tiles_y,pods_x,pods_y) in cores:
            print(test_str(inp,tiles_x,tiles_y,pods_x,pods_y,grain))
