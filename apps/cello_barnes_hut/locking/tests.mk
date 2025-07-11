
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[nbodies],[tiles-x],[tiles-y],[pods-x],[pods-y],[exponential_backoff],[try_lock],[run])

TESTS += $(call test-name,65536,16,8,4,2,no,no,0)
TESTS += $(call test-name,65536,16,8,4,2,no,yes,0)
TESTS += $(call test-name,65536,16,8,4,2,yes,no,0)
TESTS += $(call test-name,65536,16,8,4,2,yes,yes,0)
