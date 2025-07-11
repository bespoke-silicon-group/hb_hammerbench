def test_str(num_iter, tiles_x, tiles_y, pods_x, pods_y, exponential_backoff, try_lock, run):
    return f'TESTS += $(call test-name,{num_iter},{tiles_x},{tiles_y},{pods_x},{pods_y},{exponential_backoff},{try_lock},{run})'

header = """
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[num-iter],[tiles-x],[tiles-y],[pods-x],[pods-y],[exponential_backoff],[try_lock],[run])
"""

inputs = [64*1024]

options = [
    ("no", "no"),
    ("no", "yes"),    
    ("yes", "no"),
    ("yes", "yes"),
]

cores = [
    (16,8,4,2)
]

runs = 1

print(header)

for run in range(runs):
    for exponential_backoff, try_lock in options:
        for inp in inputs:
            for (tiles_x,tiles_y,pods_x,pods_y) in cores:
                print(test_str(inp, tiles_x, tiles_y, pods_x, pods_y, exponential_backoff, try_lock, run))
