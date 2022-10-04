################################################################
# CHANGE ME: TESTS				               #
# TESTS += $(call test-name,[pull/push],[graph name],[pod-id]) #
################################################################
#TESTS += $(call test-name,pull,wiki-Vote,1)
#TESTS += $(call test-name,pull,wiki-Vote,2)
TESTS +=$(foreach i,$(shell seq -s" " 0 63),$(call test-name,pull,wiki-Vote,$(i)))
TESTS +=$(foreach i,$(shell seq -s" " 0 63),$(call test-name,pull,p2p-Gnutella09,$(i)))
TESTS +=$(foreach i,$(shell seq -s" " 0 63),$(call test-name,pull,email-Enron,$(i)))
TESTS +=$(foreach i,$(shell seq -s" " 0 63),$(call test-name,pull,cit-HepTh,$(i)))
TESTS +=$(foreach i,$(shell seq -s" " 0 63),$(call test-name,pull,rgg_n_2_15_s0,$(i)))

#TESTS += $(call test-name,push,wiki-Vote,2)

# TODO: Range function
