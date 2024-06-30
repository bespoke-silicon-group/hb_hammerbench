POD_RANGE = $(shell seq -s " " 0 63)

# TEST graph
#TESTS += $(call test-name,u16k16,0)

# soc-Pokec
#$(foreach x, $(POD_RANGE), $(eval TESTS += $(call test-name,soc-Pokec,$(x))))
# LJ
#$(foreach x, $(POD_RANGE), $(eval TESTS += $(call test-name,ljournal-2008,$(x))))
# HW
#$(foreach x, $(POD_RANGE), $(eval TESTS += $(call test-name,hollywood-2009,$(x))))
# CA
#$(foreach x, $(POD_RANGE), $(eval TESTS += $(call test-name,roadNet-CA,$(x))))
# US
#$(foreach x, $(POD_RANGE), $(eval TESTS += $(call test-name,road-usa,$(x))))
# RC
#$(foreach x, $(POD_RANGE), $(eval TESTS += $(call test-name,road-central,$(x))))
# WV
#$(foreach x, $(POD_RANGE), $(eval TESTS += $(call test-name,wiki-Vote,$(x))))


# known-slow pods;
TESTS += $(call test-name,soc-Pokec,0)
TESTS += $(call test-name,hollywood-2009,0)
TESTS += $(call test-name,ljournal-2008,0)
TESTS += $(call test-name,roadNet-CA,0)
TESTS += $(call test-name,road-central,0)
TESTS += $(call test-name,road-usa,0)
TESTS += $(call test-name,wiki-Vote,0)
