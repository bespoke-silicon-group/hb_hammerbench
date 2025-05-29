def test_str(vsize, compute, tiles_x, tiles_y, pods_x, pods_y, run):
    return f'TESTS += $(call test-name,{vsize},{compute},{tiles_x},{tiles_y},{pods_x},{pods_y},{run})'

header = """
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[vsize],[compute],[tiles-x],[tiles-y],[pods-x],[pods-y],[run])
"""

#inputs = [(8*1024, 10**e) for e in (0, 1, 2, 3, 4, 5, 6)]
inputs = [(1024, 10**e) for e in (0,)]

cores = [( 1,1,1,1),
         ( 2,1,1,1),                           
         ( 2,2,1,1),                  
         ( 4,2,1,1),         
         ( 4,4,1,1),
         ( 8,4,1,1),         
         ( 8,8,1,1),
         (16,8,1,1)]

runs = 10

print(header)

for run in range(runs):
    for (vsize, compute) in inputs:
        for (tiles_x,tiles_y,pods_x,pods_y) in cores:
            print(test_str(vsize, compute, tiles_x, tiles_y, pods_x, pods_y, run))

