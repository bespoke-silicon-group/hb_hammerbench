def test_str(mtx_a, mtx_b, tiles_x, tiles_y, pods_x, pods_y, exponential_backoff, try_lock, run):
    return f'TESTS += $(call test-name,{mtx_a},{mtx_b},{tiles_x},{tiles_y},{pods_x},{pods_y},{exponential_backoff},{try_lock},{run})'

header = """
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[mtx-a],[mtx-b],[tiles-x],[tiles-y],[pods-x],[pods-y],[exponential_backoff],[try_lock],[run])
"""

inputs = [
    ("email-Enron","email-Enron"),
    ("roadNet-CA","roadNet-CA"),
]

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
        for (mtx_a, mtx_b) in inputs:
            for (tiles_x,tiles_y,pods_x,pods_y) in cores:
                print(test_str(mtx_a, mtx_b, tiles_x, tiles_y, pods_x, pods_y, exponential_backoff, try_lock, run))
