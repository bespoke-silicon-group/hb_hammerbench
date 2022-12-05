import sys
import pandas as pd
df = pd.read_csv(sys.argv[1])
df = df.groupby(['nbodies', 'dimx',"dimy"])['Abs Total Cycles'].max()
df.to_csv(sys.argv[1])
