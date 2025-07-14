####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[num-seq])
#TESTS += $(call test-name,512)
TESTS += $(call test-name,$(shell echo 1024*1024|bc))


