import dram_utilization as dr
import vcache_utilization as vc
import core_utilization as core

import os
import sys

os.chdir(sys.argv[1])

dram_stat = dr.parse_dram_stat()
dr.print_dram_stat(dram_stat)

vc_stat = vc.parse_vcache_stat()
vc.print_vcache_stat(vc_stat)

core_stat = core.parse_vanilla_stat()
core.print_vanilla_stat(core_stat)
