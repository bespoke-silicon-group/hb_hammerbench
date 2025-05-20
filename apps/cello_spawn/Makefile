HB_HAMMERBENCH_PATH = $(shell git rev-parse --show-toplevel)

.PHONY: all
all: generate

# call to generate a test name
test-name = n_$(1)__size_$(2)__dmem_$(3)

# call to get parameter from test name
get-n = $(lastword $(subst _, ,$(filter n_%,$(subst __, ,$(1)))))
get-size = $(lastword $(subst _, ,$(filter size_%,$(subst __, ,$(1)))))
get-dmem = $(lastword $(subst _, ,$(filter dmem_%,$(subst __, ,$(1)))))

# defines tests
TESTS =
include tests.mk

TESTS_DIRS = $(TESTS)

$(addsuffix /parameters.mk,$(TESTS_DIRS)): %/parameters.mk:
	@echo Creating $@
	@mkdir -p $(dir $@)
	@touch $@
	@echo test-name  = $* >> $@
	@echo n = $(call get-n,$*) >> $@
	@echo size = $(call get-size,$*) >> $@
	@echo dmem = $(call get-dmem,$*) >> $@

include $(HB_HAMMERBENCH_PATH)/mk/testbench_common.mk


