####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[n],[tiles-x],[tiles-y],[pods-x],[pods-y])
TESTS += $(call test-name,$(shell echo 1024|bc),16,8,2,2)


