
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[nbodies],[tiles-x],[tiles-y],[pods-x],[pods-y])

TESTS += $(call test-name,1048576,1,1,1,1)
TESTS += $(call test-name,1048576,1,1,2,1)
TESTS += $(call test-name,1048576,1,1,2,2)
TESTS += $(call test-name,1048576,1,1,4,2)
TESTS += $(call test-name,1048576,2,1,4,2)
TESTS += $(call test-name,1048576,2,2,4,2)
TESTS += $(call test-name,1048576,4,2,4,2)
TESTS += $(call test-name,1048576,4,4,4,2)
TESTS += $(call test-name,1048576,8,4,4,2)
TESTS += $(call test-name,1048576,8,8,4,2)
TESTS += $(call test-name,1048576,16,8,4,2)
