HB_HAMMERBENCH_PATH = $(shell git rev-parse --show-toplevel)

.PHONY: all
all: generate

# call to generate a test name
test-name = loads_$(1)__tx_$(2)__ty_$(3)

# call to get parameter from test name
get-loads = $(lastword $(subst _, ,$(filter loads_%,$(subst __, ,$(1)))))
get-tx = $(lastword $(subst _, ,$(filter tx_%,$(subst __, ,$(1)))))
get-ty = $(lastword $(subst _, ,$(filter ty_%,$(subst __, ,$(1)))))

# defines tests
TESTS =
include tests.mk

TESTS_DIRS = $(TESTS)

$(addsuffix /parameters.mk,$(TESTS_DIRS)): %/parameters.mk:
	@echo Creating $@
	@mkdir -p $(dir $@)
	@touch $@
	@echo test-name  = $* >> $@
	@echo loads = $(call get-loads,$*) >> $@
	@echo tx = $(call get-tx,$*) >> $@
	@echo ty = $(call get-ty,$*) >> $@

include $(HB_HAMMERBENCH_PATH)/mk/testbench_common.mk

