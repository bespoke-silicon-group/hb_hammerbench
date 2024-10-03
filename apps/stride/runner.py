import subprocess
import matplotlib as mpl
mpl.use('pdf')
import itertools
from matplotlib import pyplot as plt
import pandas as pd
import numpy as np
class Test(object):
    def __init__(self, stride_size, stride_n, x, y, warmup, tiles_x, tiles_y):
        self.stride_size = stride_size
        self.stride_n = stride_n
        self.x = x
        self.y = y
        self.warmup = warmup
        self.tiles_x = tiles_x
        self.tiles_y = tiles_y
        self._data = None
        self._dram_data = None
    
    @property
    def data(self):
        if self._data is None:
            self._data = self.vanilla_stats_data()
            self._data = self._data[self._data['x']==self.x]
            self._data = self._data[self._data['y']==self.y]
        return self._data


    @property
    def dram_data(self):
        if self._dram_data is None:
            self._dram_data = self.dramsim3_stats_data().transpose()
        return self._dram_data
    
    def format_for_tests_mk(self):
        return "TESTS += $(call test-name,{},{},{},{},{},{},{})".format(
            self.stride_size, self.stride_n, self.x, self.y, self.warmup, self.tiles_x, self.tiles_y
        )

    def run_directory(self):
        return "stride-size_{}__stride-n_{}__x_{}__y_{}__warmup_{}__tiles-x_{}__tiles-y_{}".format(
            self.stride_size, self.stride_n, self.x, self.y, self.warmup, self.tiles_x, self.tiles_y
        )

    def vanilla_stats_csv(self):
        return self.run_directory() + "/vanilla_stats.csv"

    def dramsim3_json(self):
        return self.run_directory() + "/dramsim3.json"

    def dramsim3_stats_data(self):
        return pd.read_json(self.dramsim3_json())
    
    def vanilla_stats_data(self):
        return pd.read_csv(self.vanilla_stats_csv())

    def cycles(self):        
        return self.data.diff()['cycle'].sum()

    def dram_cycles(self):
        return self.dram_data['average_read_latency'].sum()

def testbench(warmup, tiles_x, tiles_y):
    # create tests.mk
    STRIDE_SIZES = [32]
    STRIDE_NS = [1000, 1100, 1200, 1300, 1400, 1500, 1600, 1700, 1800, 1900]
    # randomly pick 10 (x,y) pairs from the 16x8 grid
    XYS = itertools.product(range(tiles_x), range(tiles_y))
    XYS = list(XYS)
    np.random.seed(0)
    np.random.shuffle(XYS)
    XYS = XYS[:10]
    TESTS = [Test(size, n, x, y, warmup, tiles_x, tiles_y) for (size, n, (x, y)) in itertools.product(STRIDE_SIZES, STRIDE_NS, XYS)]
    with open("tests.mk", "w") as tests_mk:
        for test in TESTS:
            tests_mk.write(test.format_for_tests_mk() + '\n')


    # run the tests using `make && make profile -j` command
    # show stdout and stderr
    subprocess.run(['make'], shell=True, check=True)
    subprocess.run(['make -C {} machine -j'.format(TESTS[0].run_directory())], shell=True, check=True)
    subprocess.run(['make && make profile -j 31'], shell=True, check=True)

    print("Tests completed")
    
    # plot the results
    # plot each (x,y) pair on its own plot
    # x-axis is stride-n, y-axis is cycles
    dram_cycles = []
    cycles = {(x,y) : [] for (x,y) in XYS}
    stride_n = {(x,y) : [] for (x,y) in XYS}
    for test in TESTS:
        dram_cycles.append(test.dram_cycles())
        cycles[(test.x, test.y)].append(test.cycles())
        stride_n[(test.x, test.y)].append(test.stride_n)

    FITS = {}
    for (x,y) in XYS:
        fit = np.polyfit(stride_n[(x,y)], cycles[(x,y)], 1)
        FITS[(x,y)] = fit
        plt.plot(stride_n[(x,y)], cycles[(x,y)], 'x')
        plt.plot(stride_n[(x,y)], np.polyval(fit, stride_n[(x,y)]), label="x={} ,y={}: {:.2f}x + {:.2f}".format(x,y,*fit))
        for (n, c) in zip(stride_n[(x,y)], cycles[(x,y)]):
            print("x={}, y={}, n={}, c={}, c/n={}".format(x, y, n, c, c/n))

    print("Tiles: {}x{}".format(tiles_x, tiles_y))
    print("mean total latency (core cycles): {}".format(np.average([fit[0] for fit in FITS.values()])))
    print("mean dram latency (dram cycles): {}".format(np.average(dram_cycles)))
    plt.title('{}x{} Cycles vs Stride-n'.format(tiles_x, tiles_y))
    plt.xlabel('stride-n')
    plt.ylabel('cycles')
    plt.legend()
    plt.savefig('cycles_vs_stride-n_{}_{}x{}.png'.format(
        "warm" if warmup == "yes" else "cold", tiles_x, tiles_y
    ))
    plt.cla()

def debug():
    t = Test(32, 1000, 12, 0, "no")
    print(t.dram_cycles())

if __name__ == "__main__":
    debug()
