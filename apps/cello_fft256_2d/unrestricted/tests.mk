
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[num-iter],[tiles-x],[tiles-y],[pods-x],[pods-y],[unrestricted])

TESTS += $(call test-name,16,16,8,4,2,no)
TESTS += $(call test-name,16,16,8,4,2,yes)
