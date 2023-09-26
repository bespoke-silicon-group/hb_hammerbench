POD_RANGE = $(shell seq -s " " 0 63)

# wiki-Vote
$(foreach x, $(POD_RANGE), $(eval TESTS += $(call test-name,wiki-Vote,$(x))))
# offshore
# road central
# roadNet CA
