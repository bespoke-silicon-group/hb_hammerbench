
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[n],[task_size],[opt_memcpy],[opt_pod_address],[tiles-x],[tiles-y],[pods-x],[pods-y],[run])

TESTS += $(call test-name,9000,32,yes,yes,16,8,4,2,0)
TESTS += $(call test-name,9000,32,yes,no,16,8,4,2,0)
TESTS += $(call test-name,9000,32,no,yes,16,8,4,2,0)
TESTS += $(call test-name,9000,32,no,no,16,8,4,2,0)
