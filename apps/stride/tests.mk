####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[n],[stride-size],[vector-size],[warm-cache],[tile-x],[tile-y],[pod-x],[pod-y])
TESTS += $(call test-name,32,16,$(shell echo 16*4*2|bc),yes,0,0,0,0)
TESTS += $(call test-name,64,16,$(shell echo 16*4*2|bc),yes,0,0,0,0)
TESTS += $(call test-name,96,16,$(shell echo 16*4*2|bc),yes,0,0,0,0)
TESTS += $(call test-name,128,16,$(shell echo 16*4*2|bc),yes,0,0,0,0)
TESTS += $(call test-name,32,16,$(shell echo 1024*1024|bc),no,0,0,0,0)
TESTS += $(call test-name,64,16,$(shell echo 1024*1024|bc),no,0,0,0,0)
TESTS += $(call test-name,96,16,$(shell echo 1024*1024|bc),no,0,0,0,0)
TESTS += $(call test-name,128,16,$(shell echo 1024*1024|bc),no,0,0,0,0)

