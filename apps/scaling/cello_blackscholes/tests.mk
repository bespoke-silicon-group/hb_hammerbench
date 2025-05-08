####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[num_options],[tiles-x],[tiles-y],[pods-x],[pods-y])
TESTS += $(call test-name,$(shell echo 1024*1024|bc),1,1,1,1)
TESTS += $(call test-name,$(shell echo 1024*1024|bc),1,1,2,1)
TESTS += $(call test-name,$(shell echo 1024*1024|bc),1,1,2,2)
TESTS += $(call test-name,$(shell echo 1024*1024|bc),1,1,4,2)


