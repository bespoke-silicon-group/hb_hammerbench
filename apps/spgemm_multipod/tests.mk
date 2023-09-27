POD_RANGE = $(shell seq -s " " 0 0)

# wiki-Vote
$(foreach x, $(POD_RANGE), $(eval TESTS += $(call test-name,wiki-Vote,$(x))))
# offshore
$(foreach x, $(POD_RANGE), $(eval TESTS += $(call test-name,offshore,$(x))))
# road central
$(foreach x, $(POD_RANGE), $(eval TESTS += $(call test-name,road-central,$(x))))
# roadNet CA
$(foreach x, $(POD_RANGE), $(eval TESTS += $(call test-name,roadNet-CA,$(x))))
