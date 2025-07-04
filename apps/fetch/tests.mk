####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[size],[n])
#TESTS += $(call test-name,$(shell echo 1024|bc),$(shell echo 128|bc))
TESTS += $(call test-name,$(shell echo 1024*32|bc),$(shell echo 1024|bc))
#TESTS += $(call test-name,$(shell echo 16*1024*32|bc),$(shell echo 16*1024|bc))


