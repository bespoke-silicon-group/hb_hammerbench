def test_str(num_iter, tiles_x, tiles_y, pods_x, pods_y, grain_scale):
    return f'TESTS += $(call test-name,{num_iter},{tiles_x},{tiles_y},{pods_x},{pods_y},{grain_scale})'

header = """
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[num-iter],[tiles-x],[tiles-y],[pods-x],[pods-y],[grain])
"""

grains =  [1,2,3,4,5,6,7,8]
inputs = [32*2*1024]

cores = [
    (1,1,4,2),
    (2,1,4,2),
    (2,2,4,2),
    (4,2,4,2),
    (4,4,4,2),
    (8,4,4,2),
    (8,8,4,2),
    (16,8,4,2)
]

print(header)

for grain in grains:
    for inp in inputs:
        for (tiles_x,tiles_y,pods_x,pods_y) in cores:
            print(test_str(inp,tiles_x,tiles_y,pods_x,pods_y, grain))
