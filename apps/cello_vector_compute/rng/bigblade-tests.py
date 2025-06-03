def test_str(vsize, compute, tiles_x, tiles_y, pods_x, pods_y, thief_rng, run):
    return f'TESTS += $(call test-name,{vsize},{compute},{tiles_x},{tiles_y},{pods_x},{pods_y},{thief_rng},{run})'

header = """
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[vsize],[compute],[tiles-x],[tiles-y],[pods-x],[pods-y],[thief-rng],[run])
"""

inputs = [(8*1024, 10**e) for e in (3,)]
#inputs = [(1024, 10**e) for e in (0,)]

cores = [(16,8,4,2)]

runs = 10

print(header)

for rng in ("xorshift","linearcong"):
    for run in range(runs):
        for (vsize, compute) in inputs:
            for (tiles_x,tiles_y,pods_x,pods_y) in cores:
                print(test_str(vsize, compute, tiles_x, tiles_y, pods_x, pods_y, rng, run))

