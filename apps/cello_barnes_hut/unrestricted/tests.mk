
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[nbodies],[tiles-x],[tiles-y],[pods-x],[pods-y],[unrestricted])

TESTS += $(call test-name,4096,16,8,4,2,no)
TESTS += $(call test-name,4096,16,8,4,2,yes)
