#########################################################
# CHANGE ME: TESTS				        #
# TESTS += $(call test-name,[buffer-size],[warm-cache]) #
#########################################################
#MSIZE = 524288
MSIZE = 4096
TESTS += $(call test-name,4,4,$(MSIZE),no)
