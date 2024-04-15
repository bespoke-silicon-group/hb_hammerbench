POD_RANGE = $(shell seq -s " " 0 63)

# test
#TESTS += $(call test-name,test4,0)
#TESTS += $(call test-name,test8,0)

# wiki-Vote
#$(foreach x, $(POD_RANGE), $(eval TESTS += $(call test-name,wiki-Vote,$(x))))
# offshore
#$(foreach x, $(POD_RANGE), $(eval TESTS += $(call test-name,offshore,$(x))))
# road central
#$(foreach x, $(POD_RANGE), $(eval TESTS += $(call test-name,road-central,$(x))))
# roadNet CA
#$(foreach x, $(POD_RANGE), $(eval TESTS += $(call test-name,roadNet-CA,$(x))))


# known-slow pods;
#TESTS += $(call test-name,wiki-Vote,0)
#TESTS += $(call test-name,offshore,51)
#TESTS += $(call test-name,roadNet-CA,18)
#TESTS += $(call test-name,road-central,16)


#TESTS += $(call test-name,road-central,16,1,1,1)
#TESTS += $(call test-name,road-central,16,2,1,2)
#TESTS += $(call test-name,road-central,16,4,1,3)
#TESTS += $(call test-name,road-central,16,8,1,4)
#TESTS += $(call test-name,road-central,16,16,1,5)
#TESTS += $(call test-name,road-central,16,16,2,6)
#TESTS += $(call test-name,road-central,16,16,4,7)
#TESTS += $(call test-name,road-central,16,16,8,8)

TESTS += $(call test-name,wiki-Vote,0,1,8)
TESTS += $(call test-name,wiki-Vote,0,2,7)
TESTS += $(call test-name,wiki-Vote,0,4,6)
TESTS += $(call test-name,wiki-Vote,0,8,5)
#TESTS += $(call test-name,test8,0,1,8)
#TESTS += $(call test-name,test8,0,2,7)
#TESTS += $(call test-name,test8,0,4,6)
#TESTS += $(call test-name,test8,0,8,5)
