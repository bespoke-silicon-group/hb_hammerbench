import sys
import pandas as pd
import numpy as np
from tag import Tag

def fix_dramsim3_json(filename_i, filename_o):
    jsonfile_o = open(filename_o, "w")
    with open(filename_i, "r") as jsonfile:
        lines = [line for line in jsonfile]

    for (lno, line) in enumerate(lines):
        # if this is the last line
        # replace the ',' with ']'
        if lno == len(lines)-1:
            line = line.replace(',\n',']\n')
        # write it out
        jsonfile_o.write(line)

class DRAMSim3Stats(object):
    """
    wrapper for DRAMSim3 stats object
    """
    def __init__(self, json_filename, tail=0.0):
        """
        Initialize from a JSON file from DRAMSim3
        """
        self._filename = json_filename + ".fixed.json"        
        fix_dramsim3_json(json_filename, self._filename)

        # setup the "groupby" fields
        self._group_by_fields = ["tag"]
                
        # read the json file
        self._data = pd.read_json(self._filename)
        self._data['tag_type']=self._data['tag'].map(lambda tag : Tag(tag).type)
        self._subclass_init_add_group_by_fields()

        # only keep the min and max cycles
        # lop off the tail data
        # this discounts cycles where < tail% of tiles are still running
        self._kstart_data = self._data[self._data['tag_type'].str.match('kernel_start')]
        self._kend_data   = self._data[self._data['tag_type'].str.match('kernel_end')]

        lbound=self._kstart_data['num_cycles'].quantile(tail)
        ubound=self._kend_data['num_cycles'].quantile(1-tail)

        # take our min and max cycle
        self._min_cycle=self._data[self._data['num_cycles']>=lbound]['num_cycles'].min()
        self._max_cycle=self._data[self._data['num_cycles']<=ubound]['num_cycles'].max()

        # filter only the data in target time range
        filtered_data = self._data[(self._data["num_cycles"]==self._min_cycle)|
                                   (self._data["num_cycles"]==self._max_cycle)]        
        
        filtered_data = filtered_data.drop([
            'act_stb_energy',
            'all_bank_idle_cycles',
            'pre_stb_energy',
            'rank_active_cycles',
            'sref_cycles',
            'sref_energy',            
        ], 1)        
        
        # do a diff by channel
        # bug here! num_cycles gets summed
        self._diffed_data = filtered_data.groupby(self._group_by_fields).sum().diff().dropna()
        self._diffed_data['cycles'] = self._diffed_data['num_cycles']/8

        self._epoch_data = filtered_data.groupby(['num_cycles'] + self._group_by_fields).sum().diff().dropna()
        
    def _subclass_init_add_group_by_fields(self):
        """
        This should be overridden by a subclass, if applicable
        Adds fields by which to group to produce diffed data
        """
        return
    
    def __str__(self):
        """
        Format as string
        """
        return str(self._data)

    @property
    def filename(self):
        return self._filename
    
    @property
    def fields(self):
        """
        Returns the column names in a list
        """
        return self._data.columns.to_list()

    @property
    def data(self):
        return self._data

    @property
    def diffed_data(self):
        return self._diffed_data

    @property
    def epoch_data(self):
        return self._epoch_data
