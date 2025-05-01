####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[type],[size])
TESTS += $(call test-name,read,$(shell echo 128*16*8|bc))


