HB_HAMMERBENCH_PATH = $(shell git rev-parse --show-toplevel)

.PHONY: all
all: generate

# call to generate a test name
test-name = num-seq_$(1)__opt-memcpy_$(2)__opt-restrict-ws_$(3)__opt-lock_$(4)__run_$(5)

# call to get parameter from test name
get-num-seq = $(lastword $(subst _, ,$(filter num-seq_%,$(subst __, ,$(1)))))
get-opt-memcpy = $(lastword $(subst _, ,$(filter opt-memcpy_%,$(subst __, ,$(1)))))
get-opt-restrict-ws = $(lastword $(subst _, ,$(filter opt-restrict-ws_%,$(subst __, ,$(1)))))
get-opt-lock = $(lastword $(subst _, ,$(filter opt-lock_%,$(subst __, ,$(1)))))
get-run = $(lastword $(subst _, ,$(filter run_%,$(subst __, ,$(1)))))

# defines tests
TESTS =
include tests.mk

TESTS_DIRS = $(TESTS)

$(addsuffix /parameters.mk,$(TESTS_DIRS)): %/parameters.mk:
	@echo Creating $@
	@mkdir -p $(dir $@)
	@touch $@
	@echo test-name  = $* >> $@
	@echo num-seq = $(call get-num-seq,$*) >> $@
	@echo opt-memcpy = $(call get-opt-memcpy,$*) >> $@
	@echo opt-restrict-ws = $(call get-opt-restrict-ws,$*) >> $@
	@echo opt-lock = $(call get-opt-lock,$*) >> $@
	@echo run = $(call get-run,$*) >> $@

include $(HB_HAMMERBENCH_PATH)/mk/testbench_common.mk


