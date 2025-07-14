####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[num-option])
#TESTS += $(call test-name,$(shell echo 128|bc))
TESTS += $(call test-name,$(shell echo 64*1024|bc))


