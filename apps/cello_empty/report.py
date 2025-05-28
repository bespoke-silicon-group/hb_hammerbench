import sys
import pandas as pd
from benchmark_dir_parse import *

fields = ['none']

stats = [
    # add extra stats to sum like 'instr_addi' e.g.
]

cello_stats = [
    ('cello_steals', 'sum'),
    ('cello_task_execute', 'sum'),
    ('cello_task_execute', 'min'),
    ('cello_task_execute', 'max'),
    ('cello_task_push', 'sum'),
    ('cello_task_push', 'min'),
    ('cello_task_push', 'max'),
    ('cello_owner_lock_acquire_fail', 'sum'),
    ('cello_owner_lock_acquire_fail', 'min'),
    ('cello_owner_lock_acquire_fail', 'max'),
]

df = pd.DataFrame()
for testdir in sys.argv[1:]:
    df = df.append(testdir_parse(testdir, fields, stats, cello_stats = cello_stats))

df.to_csv('report.csv')

