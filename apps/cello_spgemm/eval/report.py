import sys
import pandas as pd
from benchmark_dir_parse import *

fields = ['mtx-a', 'mtx-b', 'run']

stats = [
    # add extra stats to sum like 'instr_addi' e.g.
]

df = pd.DataFrame()
for testdir in sys.argv[1:]:
    df = df.append(testdir_parse(testdir, fields, stats))

df.to_csv('report.csv')

