################################################################
# CHANGE ME: TESTS				               #
# TESTS += $(call test-name,[pull/push],[graph name],[pod-id]) #
################################################################
# These tests run relatively quickly:
# Social network-like

TESTS +=$(foreach i,$(shell seq -s" " 0 63),$(call test-name,pull,pagerank_pull_u8,wiki-Vote,cyclic,$(i),64))
TESTS +=$(foreach i,$(shell seq -s" " 0 48),$(call test-name,pull,pagerank_pull_u8,wiki-Vote,cyclic,$(i),49))
TESTS +=$(foreach i,$(shell seq -s" " 0 35),$(call test-name,pull,pagerank_pull_u8,wiki-Vote,cyclic,$(i),36))
TESTS +=$(foreach i,$(shell seq -s" " 0 24),$(call test-name,pull,pagerank_pull_u8,wiki-Vote,cyclic,$(i),25))
TESTS +=$(foreach i,$(shell seq -s" " 0 15),$(call test-name,pull,pagerank_pull_u8,wiki-Vote,cyclic,$(i),16))
TESTS +=$(foreach i,$(shell seq -s" " 0 8),$(call test-name,pull,pagerank_pull_u8,wiki-Vote,cyclic,$(i),9))
TESTS +=$(foreach i,$(shell seq -s" " 0 3),$(call test-name,pull,pagerank_pull_u8,wiki-Vote,cyclic,$(i),4))
TESTS +=$(foreach i,$(shell seq -s" " 0 0),$(call test-name,pull,pagerank_pull_u8,wiki-Vote,cyclic,$(i),1))

# Scientific
TESTS +=$(foreach i,$(shell seq -s" " 0 48),$(call test-name,pull,pagerank_pull_u8,offshore,cyclic,$(i),49))
TESTS +=$(foreach i,$(shell seq -s" " 0 63),$(call test-name,pull,pagerank_pull_u8,offshore,cyclic,$(i),64))
TESTS +=$(foreach i,$(shell seq -s" " 0 35),$(call test-name,pull,pagerank_pull_u8,offshore,cyclic,$(i),36))
TESTS +=$(foreach i,$(shell seq -s" " 0 24),$(call test-name,pull,pagerank_pull_u8,offshore,cyclic,$(i),25))
TESTS +=$(foreach i,$(shell seq -s" " 0 15),$(call test-name,pull,pagerank_pull_u8,offshore,cyclic,$(i),16))
TESTS +=$(foreach i,$(shell seq -s" " 0 8),$(call test-name,pull,pagerank_pull_u8,offshore,cyclic,$(i),9))
TESTS +=$(foreach i,$(shell seq -s" " 0 3),$(call test-name,pull,pagerank_pull_u8,offshore,cyclic,$(i),4))
TESTS +=$(foreach i,$(shell seq -s" " 0 0),$(call test-name,pull,pagerank_pull_u8,offshore,cyclic,$(i),1))

# Planar
# TESTS +=$(foreach i,$(shell seq -s" " 0 63),$(call test-name,pull,pagerank_pull_u8,hi2010,$(i),64))
# TESTS +=$(foreach i,$(shell seq -s" " 0 48),$(call test-name,pull,pagerank_pull_u8,hi2010,$(i),49))
# TESTS +=$(foreach i,$(shell seq -s" " 0 35),$(call test-name,pull,pagerank_pull_u8,hi2010,$(i),36))
# TESTS +=$(foreach i,$(shell seq -s" " 0 24),$(call test-name,pull,pagerank_pull_u8,hi2010,$(i),25))
# TESTS +=$(foreach i,$(shell seq -s" " 0 15),$(call test-name,pull,pagerank_pull_u8,hi2010,$(i),16))
# TESTS +=$(foreach i,$(shell seq -s" " 0 8),$(call test-name,pull,pagerank_pull_u8,hi2010,$(i),9))
# TESTS +=$(foreach i,$(shell seq -s" " 0 3),$(call test-name,pull,pagerank_pull_u8,hi2010,$(i),4))
# TESTS +=$(foreach i,$(shell seq -s" " 0 0),$(call test-name,pull,pagerank_pull_u8,hi2010,$(i),1))

# TESTS +=$(foreach i,$(shell seq -s" " 0 63),$(call test-name,pull,pagerank_pull_u8,delaunay_n15,$(i),64))
# TESTS +=$(foreach i,$(shell seq -s" " 0 48),$(call test-name,pull,pagerank_pull_u8,delaunay_n15,$(i),49))
# TESTS +=$(foreach i,$(shell seq -s" " 0 35),$(call test-name,pull,pagerank_pull_u8,delaunay_n15,$(i),36))
# TESTS +=$(foreach i,$(shell seq -s" " 0 24),$(call test-name,pull,pagerank_pull_u8,delaunay_n15,$(i),25))
# TESTS +=$(foreach i,$(shell seq -s" " 0 15),$(call test-name,pull,pagerank_pull_u8,delaunay_n15,$(i),16))
# TESTS +=$(foreach i,$(shell seq -s" " 0 8),$(call test-name,pull,pagerank_pull_u8,delaunay_n15,$(i),9))
# TESTS +=$(foreach i,$(shell seq -s" " 0 3),$(call test-name,pull,pagerank_pull_u8,delaunay_n15,$(i),4))
# TESTS +=$(foreach i,$(shell seq -s" " 0 0),$(call test-name,pull,pagerank_pull_u8,delaunay_n15,$(i),1))

#Random:

# TESTS +=$(foreach i,$(shell seq -s" " 0 63),$(call test-name,pull,pagerank_pull_u8,rgg_n_2_15_s0,$(i),64))
# TESTS +=$(foreach i,$(shell seq -s" " 0 48),$(call test-name,pull,pagerank_pull_u8,rgg_n_2_15_s0,$(i),49))
# TESTS +=$(foreach i,$(shell seq -s" " 0 35),$(call test-name,pull,pagerank_pull_u8,rgg_n_2_15_s0,$(i),36))
# TESTS +=$(foreach i,$(shell seq -s" " 0 24),$(call test-name,pull,pagerank_pull_u8,rgg_n_2_15_s0,$(i),25))
# TESTS +=$(foreach i,$(shell seq -s" " 0 15),$(call test-name,pull,pagerank_pull_u8,rgg_n_2_15_s0,$(i),16))
# TESTS +=$(foreach i,$(shell seq -s" " 0 8),$(call test-name,pull,pagerank_pull_u8,rgg_n_2_15_s0,$(i),9))
# TESTS +=$(foreach i,$(shell seq -s" " 0 3),$(call test-name,pull,pagerank_pull_u8,rgg_n_2_15_s0,$(i),4))
# TESTS +=$(foreach i,$(shell seq -s" " 0 0),$(call test-name,pull,pagerank_pull_u8,rgg_n_2_15_s0,$(i),1))

# TESTS +=$(foreach i,$(shell seq -s" " 0 63),$(call test-name,pull,pagerank_pull_u8,rgg_n_2_16_s0,$(i),64))
# TESTS +=$(foreach i,$(shell seq -s" " 0 48),$(call test-name,pull,pagerank_pull_u8,rgg_n_2_16_s0,$(i),49))
# TESTS +=$(foreach i,$(shell seq -s" " 0 35),$(call test-name,pull,pagerank_pull_u8,rgg_n_2_16_s0,$(i),36))
# TESTS +=$(foreach i,$(shell seq -s" " 0 24),$(call test-name,pull,pagerank_pull_u8,rgg_n_2_16_s0,$(i),25))
# TESTS +=$(foreach i,$(shell seq -s" " 0 15),$(call test-name,pull,pagerank_pull_u8,rgg_n_2_16_s0,$(i),16))
# TESTS +=$(foreach i,$(shell seq -s" " 0 8),$(call test-name,pull,pagerank_pull_u8,rgg_n_2_16_s0,$(i),9))
# TESTS +=$(foreach i,$(shell seq -s" " 0 3),$(call test-name,pull,pagerank_pull_u8,rgg_n_2_16_s0,$(i),4))
# TESTS +=$(foreach i,$(shell seq -s" " 0 0),$(call test-name,pull,pagerank_pull_u8,rgg_n_2_16_s0,$(i),1))


# Slower Running:
# Social network-like
TESTS +=$(foreach i,$(shell seq -s" " 0 63),$(call test-name,pull,pagerank_pull_u8,ljournal-2008,cyclic,$(i),64))
TESTS +=$(foreach i,$(shell seq -s" " 0 48),$(call test-name,pull,pagerank_pull_u8,ljournal-2008,cyclic,$(i),49))
TESTS +=$(foreach i,$(shell seq -s" " 0 35),$(call test-name,pull,pagerank_pull_u8,ljournal-2008,cyclic,$(i),36))
TESTS +=$(foreach i,$(shell seq -s" " 0 24),$(call test-name,pull,pagerank_pull_u8,ljournal-2008,cyclic,$(i),25))
TESTS +=$(foreach i,$(shell seq -s" " 0 15),$(call test-name,pull,pagerank_pull_u8,ljournal-2008,cyclic,$(i),16))
TESTS +=$(foreach i,$(shell seq -s" " 0 8),$(call test-name,pull,pagerank_pull_u8,ljournal-2008,cyclic,$(i),9))
TESTS +=$(foreach i,$(shell seq -s" " 0 3),$(call test-name,pull,pagerank_pull_u8,ljournal-2008,cyclic,$(i),4))
TESTS +=$(foreach i,$(shell seq -s" " 0 0),$(call test-name,pull,pagerank_pull_u8,ljournal-2008,cyclic,$(i),1))

TESTS +=$(foreach i,$(shell seq -s" " 0 48),$(call test-name,pull,pagerank_pull_u8,hollywood-2009,cyclic,$(i),64))
TESTS +=$(foreach i,$(shell seq -s" " 0 48),$(call test-name,pull,pagerank_pull_u8,hollywood-2009,cyclic,$(i),49))
TESTS +=$(foreach i,$(shell seq -s" " 0 35),$(call test-name,pull,pagerank_pull_u8,hollywood-2009,cyclic,$(i),36))
TESTS +=$(foreach i,$(shell seq -s" " 0 24),$(call test-name,pull,pagerank_pull_u8,hollywood-2009,cyclic,$(i),25))
TESTS +=$(foreach i,$(shell seq -s" " 0 15),$(call test-name,pull,pagerank_pull_u8,hollywood-2009,cyclic,$(i),16))
TESTS +=$(foreach i,$(shell seq -s" " 0 8),$(call test-name,pull,pagerank_pull_u8,hollywood-2009,cyclic,$(i),9))
TESTS +=$(foreach i,$(shell seq -s" " 0 3),$(call test-name,pull,pagerank_pull_u8,hollywood-2009,cyclic,$(i),4))
TESTS +=$(foreach i,$(shell seq -s" " 0 0),$(call test-name,pull,pagerank_pull_u8,hollywood-2009,cyclic,$(i),1))

# Planar:
TESTS +=$(foreach i,$(shell seq -s" " 0 63),$(call test-name,pull,pagerank_pull_u8,roadNet-CA,blocked,$(i),64))
TESTS +=$(foreach i,$(shell seq -s" " 0 48),$(call test-name,pull,pagerank_pull_u8,roadNet-CA,blocked,$(i),49))
TESTS +=$(foreach i,$(shell seq -s" " 0 35),$(call test-name,pull,pagerank_pull_u8,roadNet-CA,blocked,$(i),36))
TESTS +=$(foreach i,$(shell seq -s" " 0 24),$(call test-name,pull,pagerank_pull_u8,roadNet-CA,blocked,$(i),25))
TESTS +=$(foreach i,$(shell seq -s" " 0 15),$(call test-name,pull,pagerank_pull_u8,roadNet-CA,blocked,$(i),16))
TESTS +=$(foreach i,$(shell seq -s" " 0 8),$(call test-name,pull,pagerank_pull_u8,roadNet-CA,blocked,$(i),9))
TESTS +=$(foreach i,$(shell seq -s" " 0 3),$(call test-name,pull,pagerank_pull_u8,roadNet-CA,blocked,$(i),4))
TESTS +=$(foreach i,$(shell seq -s" " 0 0),$(call test-name,pull,pagerank_pull_u8,roadNet-CA,blocked,$(i),1))

TESTS +=$(foreach i,$(shell seq -s" " 0 48),$(call test-name,pull,pagerank_pull_u8,road_central,blocked,$(i),64))
TESTS +=$(foreach i,$(shell seq -s" " 0 48),$(call test-name,pull,pagerank_pull_u8,road_central,blocked,$(i),49))
TESTS +=$(foreach i,$(shell seq -s" " 0 35),$(call test-name,pull,pagerank_pull_u8,road_central,blocked,$(i),36))
TESTS +=$(foreach i,$(shell seq -s" " 0 24),$(call test-name,pull,pagerank_pull_u8,road_central,blocked,$(i),25))
TESTS +=$(foreach i,$(shell seq -s" " 0 15),$(call test-name,pull,pagerank_pull_u8,road_central,blocked,$(i),16))
TESTS +=$(foreach i,$(shell seq -s" " 0 8),$(call test-name,pull,pagerank_pull_u8,road_central,blocked,$(i),9))
TESTS +=$(foreach i,$(shell seq -s" " 0 3),$(call test-name,pull,pagerank_pull_u8,road_central,blocked,$(i),4))
TESTS +=$(foreach i,$(shell seq -s" " 0 0),$(call test-name,pull,pagerank_pull_u8,road_central,blocked,$(i),1))

TESTS +=$(foreach i,$(shell seq -s" " 0 48),$(call test-name,pull,pagerank_pull_u8,road_usa,blocked,$(i),64))
TESTS +=$(foreach i,$(shell seq -s" " 0 48),$(call test-name,pull,pagerank_pull_u8,road_usa,blocked,$(i),49))
TESTS +=$(foreach i,$(shell seq -s" " 0 35),$(call test-name,pull,pagerank_pull_u8,road_usa,blocked,$(i),36))
TESTS +=$(foreach i,$(shell seq -s" " 0 24),$(call test-name,pull,pagerank_pull_u8,road_usa,blocked,$(i),25))
TESTS +=$(foreach i,$(shell seq -s" " 0 15),$(call test-name,pull,pagerank_pull_u8,road_usa,blocked,$(i),16))
TESTS +=$(foreach i,$(shell seq -s" " 0 8),$(call test-name,pull,pagerank_pull_u8,road_usa,blocked,$(i),9))
TESTS +=$(foreach i,$(shell seq -s" " 0 3),$(call test-name,pull,pagerank_pull_u8,road_usa,blocked,$(i),4))
TESTS +=$(foreach i,$(shell seq -s" " 0 0),$(call test-name,pull,pagerank_pull_u8,road_usa,blocked,$(i),1))

# Random:
# TESTS +=$(foreach i,$(shell seq -s" " 0 63),$(call test-name,pull,pagerank_pull,rgg_n_2_20_s0,$(i),64))
# TESTS +=$(foreach i,$(shell seq -s" " 0 63),$(call test-name,pull,pagerank_pull_u8,rgg_n_2_20_s0,$(i),64))
# TESTS +=$(foreach i,$(shell seq -s" " 0 48),$(call test-name,pull,pagerank_pull_u8,rgg_n_2_20_s0,$(i),49))
# TESTS +=$(foreach i,$(shell seq -s" " 0 35),$(call test-name,pull,pagerank_pull_u8,rgg_n_2_20_s0,$(i),36))
# TESTS +=$(foreach i,$(shell seq -s" " 0 24),$(call test-name,pull,pagerank_pull_u8,rgg_n_2_20_s0,$(i),25))
# TESTS +=$(foreach i,$(shell seq -s" " 0 15),$(call test-name,pull,pagerank_pull_u8,rgg_n_2_20_s0,$(i),16))
# TESTS +=$(foreach i,$(shell seq -s" " 0 8),$(call test-name,pull,pagerank_pull_u8,rgg_n_2_20_s0,$(i),9))
# TESTS +=$(foreach i,$(shell seq -s" " 0 3),$(call test-name,pull,pagerank_pull_u8,rgg_n_2_20_s0,$(i),4))
# TESTS +=$(foreach i,$(shell seq -s" " 0 0),$(call test-name,pull,pagerank_pull_u8,rgg_n_2_20_s0,$(i),1))


