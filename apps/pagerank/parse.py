import sys
import pandas as pd
df = pd.read_csv(sys.argv[1])
df = df.groupby(["npods", "pod"]).sum().groupby(['npods'])['Abs Total Cycles'].max()
df.to_csv(sys.argv[1])
