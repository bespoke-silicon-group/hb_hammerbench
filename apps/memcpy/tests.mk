#########################################################
# CHANGE ME: TESTS				        #
# TESTS += $(call test-name,[buffer-size],[warm-cache]) #
#########################################################
TESTS += $(call test-name,131072,yes)
TESTS += $(call test-name,262144,no)
