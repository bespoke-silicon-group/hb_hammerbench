import itertools

def test_str(n, task_size, opt_memcpy, opt_pod_address, tiles_x, tiles_y, pods_x, pods_y, run):
    return f'TESTS += $(call test-name,{n},{task_size},{opt_memcpy},{opt_pod_address},{tiles_x},{tiles_y},{pods_x},{pods_y},{run})'

header = """
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[n],[task_size],[opt_memcpy],[opt_pod_address],[tiles-x],[tiles-y],[pods-x],[pods-y],[run])
"""
testing = False

if testing:
    n_v = [9000]
    task_size_v = [32]
    opt_memcpy_v = ["yes","no"]
    opt_podaddr_v = ["yes","no"]    
    cores = [
        (16,8,4,2)
    ]
    runs = 1    
else:
    n_v = [1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000]
    task_size_v = [0, 24, 28, 32]
    opt_memcpy_v = ["yes","no"]
    opt_podaddr_v = ["yes","no"]    
    cores = [
        (16,8,4,2)
    ]
    runs = 1

print(header)

for run, n, task_size, opt_memcpy, opt_podaddr in itertools.product(range(runs), n_v, task_size_v, opt_memcpy_v, opt_podaddr_v):
    for (tiles_x,tiles_y,pods_x,pods_y) in cores:
        print(test_str(n, task_size, opt_memcpy, opt_podaddr, tiles_x, tiles_y, pods_x, pods_y, run))
