HB_HAMMERBENCH_PATH = $(shell git rev-parse --show-toplevel)

.PHONY: all
all: generate

# call to generate a test name
test-name = vsize_$(1)__compute_$(2)

# call to get parameter from test name
get-vsize = $(lastword $(subst _, ,$(filter vsize_%,$(subst __, ,$(1)))))
get-compute = $(lastword $(subst _, ,$(filter compute_%,$(subst __, ,$(1)))))

# defines tests
TESTS =
include tests.mk

TESTS_DIRS = $(TESTS)

$(addsuffix /parameters.mk,$(TESTS_DIRS)): %/parameters.mk:
	@echo Creating $@
	@mkdir -p $(dir $@)
	@touch $@
	@echo test-name  = $* >> $@
	@echo vsize = $(call get-vsize,$*) >> $@
	@echo compute = $(call get-compute,$*) >> $@

include $(HB_HAMMERBENCH_PATH)/mk/testbench_common.mk


