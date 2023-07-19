HB_HAMMERBENCH_PATH = $(shell git rev-parse --show-toplevel)

.PHONY: all
all: generate

# call to generate a test name
test-name = width_$(1)__num-iter_$(2)__warm-cache_$(3)

# call to get parameter from test name
get-width = $(lastword $(subst _, ,$(filter width_%,$(subst __, ,$(1)))))
get-num-iter = $(lastword $(subst _, ,$(filter num-iter_%,$(subst __, ,$(1)))))
get-warm-cache = $(lastword $(subst _, ,$(filter warm-cache_%,$(subst __, ,$(1)))))

# defines tests
TESTS =
include tests.mk

TESTS_DIRS = $(TESTS)

$(addsuffix /parameters.mk,$(TESTS_DIRS)): %/parameters.mk:
	@echo Creating $@
	@mkdir -p $(dir $@)
	@touch $@
	@echo test-name  = $* >> $@
	@echo width = $(call get-width,$*) >> $@
	@echo num-iter = $(call get-num-iter,$*) >> $@
	@echo warm-cache = $(call get-warm-cache,$*) >> $@

include $(HB_HAMMERBENCH_PATH)/mk/testbench_common.mk
