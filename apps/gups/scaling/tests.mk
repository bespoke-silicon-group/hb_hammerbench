####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[updates],[size],[tiles-x],[tiles-y],[pods-x],[pods-y])
TESTS += $(call test-name,262144,$(shell echo 256*1024*1024|bc),1,1,1,1)
TESTS += $(call test-name,262144,$(shell echo 256*1024*1024|bc),2,1,1,1)
TESTS += $(call test-name,262144,$(shell echo 256*1024*1024|bc),2,2,1,1)
TESTS += $(call test-name,262144,$(shell echo 256*1024*1024|bc),4,2,1,1)
TESTS += $(call test-name,262144,$(shell echo 256*1024*1024|bc),4,4,1,1)
TESTS += $(call test-name,262144,$(shell echo 256*1024*1024|bc),8,4,1,1)
TESTS += $(call test-name,262144,$(shell echo 256*1024*1024|bc),8,8,1,1)
TESTS += $(call test-name,262144,$(shell echo 256*1024*1024|bc),16,8,1,1)

