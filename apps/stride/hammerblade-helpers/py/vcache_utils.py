import pandas as pd
from tag import Tag

class VCacheStats(object):
    def __init__(self, filename, tail=0.0):        
        self._filename = filename
        self._data = pd.read_csv(self._filename)

        # tag type
        self._data['tag_type']=self._data['tag'].map(lambda tag : Tag(tag).type)

        # lop off the tail data
        # this discounts cycles where < tail% of tiles are still running
        self._kstart_data = self._data[self._data['tag_type'].str.match('kernel_start')]
        self._kend_data   = self._data[self._data['tag_type'].str.match('kernel_end')]

        lbound=self._kstart_data['time'].quantile(tail)
        ubound=self._kend_data['time'].quantile(1-tail)

        # take our min and max cycle
        min_time=self._data[self._data['time']>=lbound]['time'].min()
        max_time=self._data[self._data['time']<=ubound]['time'].max()        

        self._group_by_fields = [
            "time",
            "tag",
        ]
        self._subclass_init_add_group_by_fields()

        # just keep the min and max time
        self._data = self._data[(self._data['time']==min_time)|(self._data['time']==max_time)]

        # get the min and max global ctr
        min_global_ctr = self._data['global_ctr'].min()
        max_global_ctr = self._data['global_ctr'].max()

        # get the diff'd data
        self._diffed_data = self._data.groupby(self._group_by_fields).sum().diff().dropna()

        # mark time
        self._diffed_data['time_start'] = min_time
        self._diffed_data['time_end'] = max_time
        self._diffed_data['time_elapsed'] = max_time-min_time

        # mark cycles
        self._diffed_data['global_ctr_start'] = min_global_ctr
        self._diffed_data['global_ctr_end'] = max_global_ctr
        self._diffed_data['global_ctr_elapsed'] = max_global_ctr-min_global_ctr

    def _subclass_init_add_group_by_fields(self):
        return
    
    @property
    def filename(self):
        return self._filename

    @property
    def data(self):
        return self._data

    @property
    def diffed_data(self):
        return self._diffed_data
    
