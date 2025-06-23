
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[num_options],[tiles-x],[tiles-y],[pods-x],[pods-y])

TESTS += $(call test-name,4096,16,8,4,2,no)
TESTS += $(call test-name,4096,16,8,4,2,yes)
