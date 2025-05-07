####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[updates],[size])
#TESTS += $(call test-name,1,1)
TESTS += $(call test-name,$(shell echo 1024*32*1|bc),$(shell echo 128*1024*1024|bc))
TESTS += $(call test-name,$(shell echo 1024*32*2|bc),$(shell echo 128*1024*1024|bc))
TESTS += $(call test-name,$(shell echo 1024*32*3|bc),$(shell echo 128*1024*1024|bc))
TESTS += $(call test-name,$(shell echo 1024*32*4|bc),$(shell echo 128*1024*1024|bc))
TESTS += $(call test-name,$(shell echo 1024*32*5|bc),$(shell echo 128*1024*1024|bc))
TESTS += $(call test-name,$(shell echo 1024*32*6|bc),$(shell echo 128*1024*1024|bc))
TESTS += $(call test-name,$(shell echo 1024*32*7|bc),$(shell echo 128*1024*1024|bc))
TESTS += $(call test-name,$(shell echo 1024*32*8|bc),$(shell echo 128*1024*1024|bc))
TESTS += $(call test-name,$(shell echo 1024*32*9|bc),$(shell echo 128*1024*1024|bc))
TESTS += $(call test-name,$(shell echo 1024*32*10|bc),$(shell echo 128*1024*1024|bc))
TESTS += $(call test-name,$(shell echo 1024*32*11|bc),$(shell echo 128*1024*1024|bc))



