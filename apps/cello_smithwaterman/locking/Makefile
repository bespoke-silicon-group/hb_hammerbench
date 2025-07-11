HB_HAMMERBENCH_PATH = $(shell git rev-parse --show-toplevel)

.PHONY: all
all: generate

# call to generate a test name
test-name = num-seq_$(1)__tiles-x_$(2)__tiles-y_$(3)__pods-x_$(4)__pods-y_$(5)__exponential-backoff_$(6)__try-lock_$(7)__run_$(8)

# call to get parameter from test name
get-num-seq = $(lastword $(subst _, ,$(filter num-seq_%,$(subst __, ,$(1)))))
get-tiles-x = $(lastword $(subst _, ,$(filter tiles-x_%,$(subst __, ,$(1)))))
get-tiles-y = $(lastword $(subst _, ,$(filter tiles-y_%,$(subst __, ,$(1)))))
get-pods-x = $(lastword $(subst _, ,$(filter pods-x_%,$(subst __, ,$(1)))))
get-pods-y = $(lastword $(subst _, ,$(filter pods-y_%,$(subst __, ,$(1)))))
get-exponential-backoff = $(lastword $(subst _, ,$(filter exponential-backoff_%,$(subst __, ,$(1)))))
get-try-lock = $(lastword $(subst _, ,$(filter try-lock_%,$(subst __, ,$(1)))))
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
	@echo tiles-x = $(call get-tiles-x,$*) >> $@
	@echo tiles-y = $(call get-tiles-y,$*) >> $@
	@echo pods-x = $(call get-pods-x,$*) >> $@
	@echo pods-y = $(call get-pods-y,$*) >> $@
	@echo exponential-backoff = $(call get-exponential-backoff,$*) >> $@
	@echo try-lock = $(call get-try-lock,$*) >> $@
	@echo run = $(call get-run,$*) >> $@

include $(HB_HAMMERBENCH_PATH)/mk/testbench_common.mk


