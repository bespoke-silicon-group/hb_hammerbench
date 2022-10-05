################################################################
# CHANGE ME: TESTS				               #
# TESTS += $(call test-name,[pull/push],[graph name],[pod-id]) #
################################################################
# These tests run relatively quickly:
# Social network-like
TESTS +=$(foreach i,$(shell seq -s" " 0 63),$(call test-name,pull,wiki-Vote,$(i),64))
TESTS +=$(foreach i,$(shell seq -s" " 0 63),$(call test-name,pull,p2p-Gnutella09,$(i),64))
TESTS +=$(foreach i,$(shell seq -s" " 0 63),$(call test-name,pull,email-Enron,$(i),64))
TESTS +=$(foreach i,$(shell seq -s" " 0 63),$(call test-name,pull,cit-HepTh,$(i),64))
TESTS +=$(foreach i,$(shell seq -s" " 0 63),$(call test-name,pull,kron_g500-logn16,$(i),64))

# Planar:
TESTS +=$(foreach i,$(shell seq -s" " 0 63),$(call test-name,pull,hi2010,$(i),64))
TESTS +=$(foreach i,$(shell seq -s" " 0 63),$(call test-name,pull,delaunay_n15,$(i),64))

#Random:
TESTS +=$(foreach i,$(shell seq -s" " 0 63),$(call test-name,pull,rgg_n_2_15_s0,$(i),64))
TESTS +=$(foreach i,$(shell seq -s" " 0 63),$(call test-name,pull,rgg_n_2_16_s0,$(i),64))

# Slower Running:
# Social network-like
# TESTS +=$(foreach i,$(shell seq -s" " 0 63),$(call test-name,pull,soc-Pokec,$(i),64))
# TESTS +=$(foreach i,$(shell seq -s" " 0 63),$(call test-name,pull,ljournal-2008,$(i),64))
# TESTS +=$(foreach i,$(shell seq -s" " 0 63),$(call test-name,pull,hollywood-2009,$(i),64))

# Planar:
# TESTS +=$(foreach i,$(shell seq -s" " 0 63),$(call test-name,pull,roadNet-CA,$(i),64))
# TESTS +=$(foreach i,$(shell seq -s" " 0 63),$(call test-name,pull,road_central,$(i),64))
# TESTS +=$(foreach i,$(shell seq -s" " 0 63),$(call test-name,pull,road_usa,$(i),64))

# Random:
# TESTS +=$(foreach i,$(shell seq -s" " 0 63),$(call test-name,pull,rgg_n_2_20_s0,$(i),64))


