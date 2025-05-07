####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[num_options])
#TESTS += $(call test-name,32)
# TESTS += $(call test-name,4096) # smallest input that 16x8 cannot do (4 per tile)
# TESTS += $(call test-name,32768)
TESTS += $(call test-name,$(shell echo 32*1024|bc))
TESTS += $(call test-name,$(shell echo 64*1024|bc))
TESTS += $(call test-name,$(shell echo 128*1024|bc))
TESTS += $(call test-name,$(shell echo 256*1024|bc))
TESTS += $(call test-name,$(shell echo 512*1024|bc))
TESTS += $(call test-name,$(shell echo 1024*1024|bc))
