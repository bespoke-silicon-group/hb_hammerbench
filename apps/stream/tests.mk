####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[type],[size])
TESTS += $(call test-name,read,$(shell echo 1024|bc))


