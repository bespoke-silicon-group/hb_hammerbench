
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[num-iter],[tiles-x],[tiles-y],[pods-x],[pods-y])

TESTS += $(call test-name,16,1,1,1,1)
TESTS += $(call test-name,16,1,1,2,1)
TESTS += $(call test-name,16,1,1,2,2)
TESTS += $(call test-name,16,1,1,4,2)
TESTS += $(call test-name,16,2,1,4,2)
TESTS += $(call test-name,16,2,2,4,2)
TESTS += $(call test-name,16,4,2,4,2)
TESTS += $(call test-name,16,4,4,4,2)
TESTS += $(call test-name,16,8,4,4,2)
TESTS += $(call test-name,16,8,8,4,2)
TESTS += $(call test-name,16,16,8,4,2)
