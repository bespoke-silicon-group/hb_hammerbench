def test_str(num_options, tiles_x, tiles_y, pods_x, pods_y, unrestricted):
    return f'TESTS += $(call test-name,{num_options},{tiles_x},{tiles_y},{pods_x},{pods_y},{unrestricted})'

header = """
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[num_options],[tiles-x],[tiles-y],[pods-x],[pods-y])
"""

inputs = [4*1024]

cores = [(16,8,4,2)]

print(header)

for unrestricted in ("no","yes"):
    for inp in inputs:
        for (tiles_x,tiles_y,pods_x,pods_y) in cores:
            print(test_str(inp,tiles_x,tiles_y,pods_x,pods_y,unrestricted))
