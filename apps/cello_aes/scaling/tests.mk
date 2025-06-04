
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[num-iter],[tiles-x],[tiles-y],[pods-x],[pods-y])

TESTS += $(call test-name,65536,1,1,1,1)
TESTS += $(call test-name,65536,1,1,2,1)
TESTS += $(call test-name,65536,1,1,2,2)
TESTS += $(call test-name,65536,1,1,4,2)
TESTS += $(call test-name,65536,2,1,4,2)
TESTS += $(call test-name,65536,2,2,4,2)
TESTS += $(call test-name,65536,4,2,4,2)
TESTS += $(call test-name,65536,4,4,4,2)
TESTS += $(call test-name,65536,8,4,4,2)
TESTS += $(call test-name,65536,8,8,4,2)
TESTS += $(call test-name,65536,16,8,4,2)
