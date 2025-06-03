import sys
import pandas as pd
from benchmark_dir_parse import *

fields = ['vsize', 'compute', 'tiles-x', 'tiles-y', 'pods-x', 'pods-y']

stats = [
    # add extra stats to sum like 'instr_addi' e.g.
]

cello_stats = [
    ('cello_owner_lock_acquire_fail','sum'),
    ('cello_owner_lock_acquire_fail','min'),
    ('cello_owner_lock_acquire_fail','max'),
    ('cello_owner_lock_acquire_fail','median'),
]

df = pd.DataFrame()
for testdir in sys.argv[1:]:
    df = df.append(testdir_parse(testdir, fields, stats, cello_stats = cello_stats))

df.to_csv('report.csv')

