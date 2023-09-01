# NUMPODS
POD_RANGE = $(shell seq -s " " 0 63)


# TEST graph u16k16
# d=1, rev_not_fwd=0, frontier_size=1, edge_traversed=32
# d=2, rev_not_fwd=0, frontier_size=32, edge_traversed=1091
# d=3, rev_not_fwd=0, frontier_size=1052, edge_traversed=34643
# d=4, rev_not_fwd=1, frontier_size=25858, edge_traversed=95775
# d=5, rev_not_fwd=1, frontier_size=38593, edge_traversed=0
#TEST_ITER_RANGE = $(shell seq -s " " 0 4)
#$(foreach n, $(TEST_ITER_RANGE), \
#	$(foreach x, $(POD_RANGE), $(eval TESTS += $(call test-name,u16k16,$(n),$(x)))))



# soc-Pokec
#PK_ITER_RANGE = $(shell seq -s " " 0 13)
#$(foreach n, $(PK_ITER_RANGE), \
	$(foreach x, $(POD_RANGE), $(eval TESTS += $(call test-name,soc-Pokec,$(n),$(x)))))

#hollywood-2009
HW_ITER_RANGE = $(shell seq -s " " 0 9)
$(foreach n, $(HW_ITER_RANGE), \
	$(foreach x, $(POD_RANGE), $(eval TESTS += $(call test-name,hollywood-2009,$(n),$(x)))))

#ljournal-2008
#TESTS += $(call test-name,ljournal-2008,0,0)

#roadnet CA
#TESTS += $(call test-name,roadNet-CA,0,0)

#road_central
#TESTS += $(call test-name,road-central,0,0)

#road_usa
#TESTS += $(call test-name,road-usa,0,0)
