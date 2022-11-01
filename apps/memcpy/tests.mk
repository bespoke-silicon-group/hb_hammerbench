#########################################################
# CHANGE ME: TESTS				        #
# TESTS += $(call test-name,[buffer-size],[warm-cache]) #
#########################################################
#TESTS += $(call test-name,131072,yes)
MSIZE = 512
#MSIZE = 522488
TESTS += $(call test-name,4,2,$(MSIZE),no)
TESTS += $(call test-name,4,4,$(MSIZE),no)
TESTS += $(call test-name,8,4,$(MSIZE),no)
TESTS += $(call test-name,8,8,$(MSIZE),no)
TESTS += $(call test-name,16,8,$(MSIZE),no)
TESTS += $(call test-name,16,16,$(MSIZE),no)
TESTS += $(call test-name,32,16,$(MSIZE),no)
