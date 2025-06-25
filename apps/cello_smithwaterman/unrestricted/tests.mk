
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[num-seq],[tiles-x],[tiles-y],[pods-x],[pods-y],[unrestricted],[run])

TESTS += $(call test-name,1048576,16,8,4,2,no,0)
TESTS += $(call test-name,1048576,16,8,4,2,yes,0)
TESTS += $(call test-name,1048576,16,8,4,2,no,1)
TESTS += $(call test-name,1048576,16,8,4,2,yes,1)
