HB_HAMMERBENCH_PATH = $(shell git rev-parse --show-toplevel)

.PHONY: all
all: generate

# call to generate a test name
test-name = num-option_$(1)__runs_$(2)

# call to get parameter from test name
get-num-option = $(lastword $(subst _, ,$(filter num-option_%,$(subst __, ,$(1)))))
get-runs = $(lastword $(subst _, ,$(filter runs_%,$(subst __, ,$(1)))))

# defines tests
TESTS =
include tests.mk

TESTS_DIRS = $(TESTS)

$(addsuffix /parameters.mk,$(TESTS_DIRS)): %/parameters.mk:
	@echo Creating $@
	@mkdir -p $(dir $@)
	@touch $@
	@echo test-name  = $* >> $@
	@echo num-option = $(call get-num-option,$*) >> $@
	@echo runs = $(call get-runs,$*) >> $@

include $(HB_HAMMERBENCH_PATH)/mk/testbench_common.mk


