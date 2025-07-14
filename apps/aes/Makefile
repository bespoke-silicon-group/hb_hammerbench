HB_HAMMERBENCH_PATH = $(shell git rev-parse --show-toplevel)

.PHONY: all
all: generate

# call to generate a test name
test-name = NUM-ITER_$(1)

# call to get parameter from test name
get-NUM-ITER = $(lastword $(subst _, ,$(filter NUM-ITER_%,$(subst __, ,$(1)))))

# defines tests
TESTS =
include tests.mk

TESTS_DIRS = $(TESTS)

$(addsuffix /parameters.mk,$(TESTS_DIRS)): %/parameters.mk:
	@echo Creating $@
	@mkdir -p $(dir $@)
	@touch $@
	@echo test-name  = $* >> $@
	@echo NUM-ITER = $(call get-NUM-ITER,$*) >> $@

include $(HB_HAMMERBENCH_PATH)/mk/testbench_common.mk


