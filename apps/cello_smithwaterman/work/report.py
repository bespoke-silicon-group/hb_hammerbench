import sys
import pandas as pd
from benchmark_dir_parse import *

fields = ['num-seq', 'tiles-x', 'tiles-y', 'pods-x', 'pods-y']

stats = [
    # add extra stats to sum like 'instr_addi' e.g.
]

cello_stats = [
    ('cello_flops','min'),
    ('cello_flops','max'),
    ('cello_flops','median'),
    ('cello_flops','mean'),
    ('cello_flops','sum'),
    ('cello_task_execute','min'),
    ('cello_task_execute','max'),
    ('cello_task_execute','median'),
    ('cello_task_execute','mean'),
    ('cello_task_execute','sum'),
    ('cello_task_push','min'),
    ('cello_task_push','max'),
    ('cello_task_push','median'),
    ('cello_task_push','mean'),
    ('cello_task_push','sum'),
    ('cello_steals','min'),
    ('cello_steals','max'),
    ('cello_steals','median'),
    ('cello_steals','mean'),
    ('cello_steals','sum'),
    ('cello_owner_lock_acquire_fail','min'),
    ('cello_owner_lock_acquire_fail','max'),
    ('cello_owner_lock_acquire_fail','median'),
    ('cello_owner_lock_acquire_fail','mean'),
    ('cello_owner_lock_acquire_fail','sum'),
]

df = pd.DataFrame()
for testdir in sys.argv[1:]:
    df = df.append(testdir_parse(testdir, fields, stats, cello_stats = cello_stats))

df.to_csv('report.csv')

