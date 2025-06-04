def test_str(num_seq, tiles_x, tiles_y, pods_x, pods_y):
    return f'TESTS += $(call test-name,{num_seq},{tiles_x},{tiles_y},{pods_x},{pods_y})'

header = """
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[num-seq],[tiles-x],[tiles-y],[pods-x],[pods-y])
"""

inputs = [32*1024]

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

for inp in inputs:
    for (tiles_x,tiles_y,pods_x,pods_y) in cores:
        print(test_str(inp,tiles_x,tiles_y,pods_x,pods_y))
