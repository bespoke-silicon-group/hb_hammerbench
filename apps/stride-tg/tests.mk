####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[n],[tile-x],[tile-y],[pod-x],[pod-y])
TESTS += $(call test-name,$(shell echo 256*1|bc),0,0,0,0)
TESTS += $(call test-name,$(shell echo 256*2|bc),0,0,0,0)
TESTS += $(call test-name,$(shell echo 256*3|bc),0,0,0,0)
TESTS += $(call test-name,$(shell echo 256*4|bc),0,0,0,0)
TESTS += $(call test-name,$(shell echo 256*5|bc),0,0,0,0)
TESTS += $(call test-name,$(shell echo 256*6|bc),0,0,0,0)


