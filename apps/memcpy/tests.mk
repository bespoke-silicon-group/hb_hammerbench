#########################################################
# CHANGE ME: TESTS				        #
# TESTS += $(call test-name,[buffer-size],[warm-cache]) #
#########################################################
TESTS += $(call test-name,512,yes)
TESTS += $(call test-name,512,no)
TESTS += $(call test-name,262144,yes)
TESTS += $(call test-name,262144,no)
