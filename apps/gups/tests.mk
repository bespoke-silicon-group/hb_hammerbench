####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[updates],[size])
#TESTS += $(call test-name,1,1)
TESTS += $(call test-name,$(shell echo 32|bc),$(shell echo 128*1024*1024|bc))


