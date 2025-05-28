import re, os
import pandas as pd
from enum import Enum


# CudaStatTag class 
# Is instantiated by a packet tag value that is recieved from a 
# bsg_cuda_print_stat(tag) insruction
# Breaks down the tag into (type, y, x, tg_id, tag>
# type of tag could be start, end, stat
# x,y are coordinates of the tile that triggered the print_stat instruciton
# tg_id is the tile group id of the tile that triggered the print_stat instruction
# Formatting for bsg_cuda_print_stat instructions
# Section                 stat type  -   y cord   -   x cord   -    tile group id   -        tag
# of bits                <----2----> -   <--6-->  -   <--6-->  -   <------14----->  -   <-----4----->
# Stat type value: {"Kernel Start":0, "Kernel End": 1, "Tag Start":2, "Tag End":3}

# The CudaStatTag class encapsulates the tag argument used by bsg_cuda_print_stat_*
# commands inside of bsg_manycore/software/bsg_manycore_lib/bsg_manycore.h.
# There are four commands:

#  bsg_cuda_print_stat_kernel_start() - Annotates the start of the kernel being profiled
#  bsg_cuda_print_stat_kernel_end()   - Annotates the end of the kernel being profiled
#  bsg_cuda_print_stat_start(tag)     - Annotates the start of a tagged section of the kernel being profiled
#  bsg_cuda_print_stat_end(tag)       - Annotates the end of a tagged section of the kernel being profiled

# Calls to bsg_cuda_print_stat_start(tag) and bsg_cuda_print_stat_kernel_start()
# must be called first be paired with a matching call to
# bsg_cuda_print_stat_end(tag) and bsg_cuda_print_stat_kernel_end().
class CudaStatTag:
    # These values are used by the manycore library in bsg_print_stat instructions
    # they are added to the tag value to determine the tile group that triggered the stat
    # and also the type of stat (stand-alone stat, start, or end)
    # the value of these paramters should match their counterpart inside 
    # bsg_manycore/software/bsg_manycore_lib/bsg_manycore.h
    _TAG_WIDTH   = 4
    _TAG_INDEX   = 0
    _TAG_MASK   = ((1 << _TAG_WIDTH) - 1)
    _TG_ID_WIDTH = 14
    _TG_ID_INDEX = _TAG_WIDTH + _TAG_INDEX
    _TG_ID_MASK = ((1 << _TG_ID_WIDTH) - 1)
    _X_WIDTH     = 6
    _X_MASK     = ((1 << _X_WIDTH) - 1)
    _X_INDEX     = _TG_ID_WIDTH + _TG_ID_INDEX
    _Y_WIDTH     = 6
    _Y_INDEX     = _X_WIDTH + _X_INDEX
    _Y_MASK     = ((1 << _Y_WIDTH) - 1)
    _TYPE_WIDTH  = 2
    _TYPE_INDEX  = _Y_WIDTH + _Y_INDEX
    _TYPE_MASK   = ((1 << _TYPE_WIDTH) - 1)

    class StatType(Enum):
        START = 0
        END = 1
        KERNEL_START   = 2
        KERNEL_END     = 3

    def __init__(self, tag):
        """ Initialize a CudaStatTag object """
        self.__s = tag;
        self.__type = self.StatType((self.__s >> self._TYPE_INDEX) & self._TYPE_MASK)

    @property
    def tag(self):
        """ Get the tag associated with this object """
        return ((self.__s >> self._TAG_INDEX) & self._TAG_MASK)

    @property
    def getTag(self):
        """ Get the tag associated with this object """
        if(self.__type == self.StatType.KERNEL_START or
           self.__type == self.StatType.KERNEL_END):
            return "Kernel"
        return ((self.__s >> self._TAG_INDEX) & self._TAG_MASK)

    @property 
    def tg_id(self):
        """ Get the Tile-Group ID associated with this object """
        return ((self.__s >> self._TG_ID_INDEX) & self._TG_ID_MASK)

    @property 
    def getTileGroupID(self):
        """ Get the Tile-Group ID associated with this object """
        return ((self.__s >> self._TG_ID_INDEX) & self._TG_ID_MASK)

    @property 
    def x(self):
        """ Get the X Coordinate associated with this object """
        return ((self.__s >> self._X_INDEX) & self._X_MASK)

    @property 
    def y(self):
        """ Get the Y Coordinate associated with this object """
        return ((self.__s >> self._Y_INDEX) & self._Y_MASK)

    @property 
    def getAction(self):
        """ Get the Action that this object defines"""
        return "Start" if self.__type in {self.StatType.KERNEL_START, self.StatType.START} else "End"

    @property 
    def statType(self):
        """ Get the StatType that this object defines"""
        return self.__type

    @property 
    def isStart(self):
        """ Return true if this object corresponds to a call to
        bsg_cuda_print_stat_start """
        return (self.__type == self.StatType.START)

    @property 
    def isEnd(self):
        """ Return true if this object corresponds to a call to
        bsg_cuda_print_stat_end """
        return (self.__type == self.StatType.END)

    @property 
    def isKernelStart(self):
        """ Return true if this object corresponds to a call to
        bsg_cuda_print_stat_kernel_start """
        return (self.__type == self.StatType.KERNEL_START)

    @property 
    def isKernelEnd(self):
        """ Return true if this object corresponds to a call to
        bsg_cuda_print_stat_kernel_end """
        return (self.__type == self.StatType.KERNEL_END)


def testdir_re_from_fields(fields):
    return  '^' + '__'.join([f'{field}_(.*?)' for field in fields]) + '$'

def testdir_get_field(folder, field):
    for s in folder.split('__'):
        m = re.match(f'^{field}_(.*)$', s)
        if m: return m.group(1)

def testdir_vanilla_dataframe(folder):
    df = pd.read_csv(f'{folder}/vanilla_stats.csv')
    return df

def testdir_cello_dataframe(folder):
    df = pd.read_csv(f'{folder}/cello_stats.csv')
    return df

def testdir_get_cycles(folder):
    dataframe = testdir_vanilla_dataframe(folder)
    dataframe['tag_type'] = dataframe['tag'].apply(lambda x : CudaStatTag(x).getAction)
    starts = dataframe[dataframe['tag_type']=='Start']
    ends = dataframe[dataframe['tag_type']=='End']
    cycles = ends['cycle'].max() - starts['cycle'].min()
    return cycles

def testdir_get_stat(folder, stat):
    dataframe = testdir_vanilla_dataframe(folder)
    dataframe['tag_type'] = dataframe['tag'].apply(lambda x : CudaStatTag(x).getAction)
    starts = dataframe[dataframe['tag_type']=='Start']
    ends = dataframe[dataframe['tag_type']=='End']
    stats = ends[stat].sum() - starts[stat].sum()
    return stats

def testdir_get_cello_stat(folder, stat, agg):    
    dataframe = testdir_cello_dataframe(folder)
    data = dataframe[stat]
    if agg == 'max':
        return data.max()
    elif agg == 'min':
        return data.min()
    elif agg == 'median':
        return data.median()    
    elif agg == 'sum':
        return data.sum()
    elif agg == 'mean':
        return data.mean()
    else:
        raise RuntimeError(f'Bad aggregator "{agg}"')
    
def testdir_get_nanoseconds(folder):
    with open(f'{folder}/kernel_ns.log','r') as ns_log:
        return int(ns_log.readline())
    
def testdir_parse(folder, fields, stats, cello_stats = []):
    df = {field : [testdir_get_field(folder, field)] for field in fields}
    # data gathered from kernel_ns.log
    try:
        df['wall_clock_ns'] = testdir_get_nanoseconds(folder)
    except FileNotFoundError as e:
        print(f'Warning: {folder}: no ns log found')

    # data gathered from vanilla_stats.csv
    try:
        df['cycles'] = testdir_get_cycles(folder)
        for stat in stats:
            df[stat] = testdir_get_stat(folder, stat)
    except FileNotFoundError as e:
        print(f'Warning: {folder}: no stats file found')

    # data gathered from cello_stats.csv
    try:
        for (stat, agg) in cello_stats:
            df[stat + '_' + agg] = testdir_get_cello_stat(folder, stat, agg)
    except FileNotFoundError as e:
        print(f'Warning: {folder}: not cello_stats file found')
    
    return pd.DataFrame(df)




