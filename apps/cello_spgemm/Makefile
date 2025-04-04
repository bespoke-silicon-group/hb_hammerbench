HB_HAMMERBENCH_PATH = $(shell git rev-parse --show-toplevel)

.PHONY: all
all: generate

# call to generate a test name
test-name = mtx-a_$(1)__mtx-b_$(2)

# call to get parameter from test name
get-mtx-a = $(lastword $(subst _, ,$(filter mtx-a_%,$(subst __, ,$(1)))))
get-mtx-b = $(lastword $(subst _, ,$(filter mtx-b_%,$(subst __, ,$(1)))))

# defines tests
TESTS =
include tests.mk

TESTS_DIRS = $(TESTS)

$(addsuffix /parameters.mk,$(TESTS_DIRS)): %/parameters.mk:
	@echo Creating $@
	@mkdir -p $(dir $@)
	@touch $@
	@echo test-name  = $* >> $@
	@echo mtx-a = $(call get-mtx-a,$*) >> $@
	@echo mtx-b = $(call get-mtx-b,$*) >> $@

include $(HB_HAMMERBENCH_PATH)/mk/testbench_common.mk
