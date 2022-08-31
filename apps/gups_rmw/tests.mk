
# Test name : (A-size, warm-cache)

# A fits in vcache
TESTS += $(call test-name,262144,yes)
# Maximum DRAM size
TESTS += $(call test-name,67108864,no)

