def test_str(mtx_a, mtx_b, tiles_x, tiles_y, pods_x, pods_y):
    return f'TESTS += $(call test-name,{mtx_a},{mtx_b},{tiles_x},{tiles_y},{pods_x},{pods_y})'

header = """
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[mtx-a],[mtx-b],[tiles-x],[tiles-y],[pods-x],[pods-y])
"""

inputs = ['u11k16', 'u12k8', 'u13k8', 'wiki-Vote', 'email-Enron', 'roadNet-CA']

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
        print(test_str(inp,inp,tiles_x,tiles_y,pods_x,pods_y))
OB
