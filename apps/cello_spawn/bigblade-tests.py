import itertools

def test_str(n, size, dmem, one_child_joiner, no_spawn, tiles_x, tiles_y, pods_x, pods_y, run):
    return f'TESTS += $(call test-name,{n},{size},{dmem},{one_child_joiner},{no_spawn},{tiles_x},{tiles_y},{pods_x},{pods_y},{run})'

header = """
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[n],[size],[dmem],[one-child-joiner],[no-spawn],[tiles-x],[tiles-y],[pods-x],[pods-y],[run])
"""

n_v = [10000, 20000, 30000, 40000, 50000, 60000, 70000, 80000, 90000]
#n_v = [10000]
task_size_v = [24]
options = [
    # dmem, one_child_joiner, no_spawn
    ("no", "no", "no"),
    ("no", "no", "yes"),
    ("no", "yes", "yes"),    
    ("yes", "no", "no"),
    ("yes", "yes", "no"),
    ("yes", "yes", "yes"),
]
cores = [
    (16,8,4,2)
]
runs = 1

print(header)

def print_tests():
    for run, n, task_size in itertools.product(range(1), n_v, task_size_v):
        for dmem, one_child_joiner, no_spawn in options:
            for (tiles_x,tiles_y,pods_x,pods_y) in cores:
                print(test_str(n, task_size, dmem, one_child_joiner, no_spawn, tiles_x, tiles_y, pods_x, pods_y, run))

print_tests()
