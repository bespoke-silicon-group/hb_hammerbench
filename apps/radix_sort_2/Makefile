HB_HAMMERBENCH_PATH = $(shell git rev-parse --show-toplevel)

.PHONY: all
all: generate

# call to generate a test name
test-name = radix-sort2-items_$(1)__unroll_$(2)__debug_$(3)__numbits_$(4)

# call to get parameter from test name
get-buffer-size = $(lastword $(subst _, ,$(filter radix-sort2-items_%,$(subst __, ,$(1)))))
get-unroll     = $(lastword $(subst _, ,$(filter unroll_%,$(subst __, ,$(1)))))
get-debug      = $(lastword $(subst _, ,$(filter debug_%,$(subst __, ,$(1)))))
get-numbits    = $(lastword $(subst _, ,$(filter numbits_%,$(subst __, ,$(1)))))

# defines tests
TESTS =
include tests.mk

TESTS_DIRS = $(TESTS)

$(addsuffix /parameters.mk,$(TESTS_DIRS)): %/parameters.mk:
	@echo Creating $@
	@mkdir -p $(dir $@)
	@touch $@
	@echo test-name  = $* >> $@
	@echo buffer-size = $(call get-buffer-size,$*) >> $@
	@echo unroll     = $(call get-unroll,$*) >> $@
	@echo debug      = $(call get-debug,$*) >> $@
	@echo numbits    = $(call get-numbits,$*) >> $@

include $(HB_HAMMERBENCH_PATH)/mk/testbench_common.mk
