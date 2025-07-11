
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[mtx-a],[mtx-b],[tiles-x],[tiles-y],[pods-x],[pods-y])

TESTS += $(call test-name,u12k16,u12k16,16,8,4,2,yes,0)
TESTS += $(call test-name,u12k16,u12k16,16,8,4,2,no,0)
TESTS += $(call test-name,u12k16,u12k16,16,8,4,2,yes,1)
TESTS += $(call test-name,u12k16,u12k16,16,8,4,2,no,1)
