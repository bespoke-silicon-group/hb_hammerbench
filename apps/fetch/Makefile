HB_HAMMERBENCH_PATH = $(shell git rev-parse --show-toplevel)

.PHONY: all
all: generate

# call to generate a test name
test-name = size_$(1)__n_$(2)

# call to get parameter from test name
get-size = $(lastword $(subst _, ,$(filter size_%,$(subst __, ,$(1)))))
get-n = $(lastword $(subst _, ,$(filter n_%,$(subst __, ,$(1)))))

# defines tests
TESTS =
include tests.mk

TESTS_DIRS = $(TESTS)

$(addsuffix /parameters.mk,$(TESTS_DIRS)): %/parameters.mk:
	@echo Creating $@
	@mkdir -p $(dir $@)
	@touch $@
	@echo test-name  = $* >> $@
	@echo size = $(call get-size,$*) >> $@
	@echo n = $(call get-n,$*) >> $@

include $(HB_HAMMERBENCH_PATH)/mk/testbench_common.mk


