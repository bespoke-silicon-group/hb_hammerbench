def test_str(num_seq, tiles_x, tiles_y, pods_x, pods_y, grain_scale):
    return f'TESTS += $(call test-name,{num_seq},{tiles_x},{tiles_y},{pods_x},{pods_y},{grain_scale})'

header = """
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[num-seq],[tiles-x],[tiles-y],[pods-x],[pods-y],[grain])
"""

inputs = [32*1024]

cores = [(16,8,4,2)]

grains = [1,2,3,4,5,6,7,8]

print(header)

for grain in grains:
    for inp in inputs:
        for (tiles_x,tiles_y,pods_x,pods_y) in cores:
            print(test_str(inp,tiles_x,tiles_y,pods_x,pods_y,grain))
