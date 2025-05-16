def test_str(vsize, compute, tiles_x, tiles_y, pods_x, pods_y):
    return f'TESTS += $(call test-name,{vsize},{compute},{tiles_x},{tiles_y},{pods_x},{pods_y})'

header = """
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[vsize],[compute],[tiles-x],[tiles-y],[pods-x],[pods-y])
"""

inputs = [(64*1024*1024, 10**e) for e in (0, 1, 2)]

cores = [( 1,1,1,1),
         ( 1,1,2,1),
         ( 1,1,2,2),
         ( 1,1,4,2),
         ( 2,1,4,2),
         ( 2,2,4,2),
         ( 4,2,4,2),
         ( 4,4,4,2),
         ( 8,4,4,2),
         ( 8,8,4,2),
         (16,8,4,2)]

print(header)

for (vsize, compute) in inputs:
    for (tiles_x,tiles_y,pods_x,pods_y) in cores:
        print(test_str(vsize, compute, tiles_x, tiles_y, pods_x, pods_y))

