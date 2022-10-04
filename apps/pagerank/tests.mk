################################################################
# CHANGE ME: TESTS				               #
# TESTS += $(call test-name,[pull/push],[graph name],[pod-id]) #
################################################################
#TESTS += $(call test-name,pull,wiki-Vote,1)
#TESTS += $(call test-name,pull,wiki-Vote,2)
TESTS +=$(foreach i,$(shell seq -s" " 0 63),$(call test-name,pull,wiki-Vote,$(i)))
#TESTS += $(call test-name,push,wiki-Vote,2)

# TODO: Range function
