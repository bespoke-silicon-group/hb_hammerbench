import pandas as pd
import sys

d = pd.read_csv(sys.argv[1])

tbl = pd.pivot_table(d, values=['operation'], index=['cycle'], columns=['operation'], aggfunc='count').fillna(0)

tbl.to_csv(sys.argv[1] + '.cycle_data.csv')
