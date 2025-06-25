
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[nbodies],[tiles-x],[tiles-y],[pods-x],[pods-y],[unrestricted],[run])

TESTS += $(call test-name,65536,16,8,4,2,yes,0)
TESTS += $(call test-name,65536,16,8,4,2,no,0)
TESTS += $(call test-name,65536,16,8,4,2,yes,1)
TESTS += $(call test-name,65536,16,8,4,2,no,1)
