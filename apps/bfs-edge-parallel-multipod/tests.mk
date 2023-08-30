# NUMPODS
POD_RANGE = $(shell seq -s " " 0 63)


# TEST graph u16k16
# d=1, rev_not_fwd=0, frontier_size=1, edge_traversed=32
# d=2, rev_not_fwd=0, frontier_size=32, edge_traversed=1091
# d=3, rev_not_fwd=0, frontier_size=1052, edge_traversed=34643
# d=4, rev_not_fwd=1, frontier_size=25858, edge_traversed=95775
# d=5, rev_not_fwd=1, frontier_size=38593, edge_traversed=0

# Iter 0
#TESTS += $(call test-name,u16k16,0,0,0)
# Iter 1
#$(foreach x, $(POD_RANGE), $(eval TESTS += $(call test-name,u16k16,0,1,$(x))))
# Iter 2
#$(foreach x, $(POD_RANGE), $(eval TESTS += $(call test-name,u16k16,0,2,$(x))))
# Iter 3
#$(foreach x, $(POD_RANGE), $(eval TESTS += $(call test-name,u16k16,0,3,$(x))))
# Iter 4
#$(foreach x, $(POD_RANGE), $(eval TESTS += $(call test-name,u16k16,0,4,$(x))))
