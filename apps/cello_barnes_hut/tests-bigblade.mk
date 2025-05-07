####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[nbodies])
#TESTS += $(call test-name,1)
#TESTS += $(call test-name,16384)
TESTS += $(call test-name,$(shell echo 16*1024|bc))
TESTS += $(call test-name,$(shell echo 32*1024|bc))
TESTS += $(call test-name,$(shell echo 64*1024|bc))
#TESTS += $(call test-name,32)
#TESTS += $(call test-name,8)


