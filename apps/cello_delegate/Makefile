HB_HAMMERBENCH_PATH = $(shell git rev-parse --show-toplevel)

.PHONY: all
all: generate

# call to generate a test name
test-name = n_$(1)__task-size_$(2)__opt-memcpy_$(3)__opt-pod-address_$(4)__tiles-x_$(5)__tiles-y_$(6)__pods-x_$(7)__pods-y_$(8)__run_$(9)

# call to get parameter from test name
get-n = $(lastword $(subst _, ,$(filter n_%,$(subst __, ,$(1)))))
get-task-size = $(lastword $(subst _, ,$(filter task-size_%,$(subst __, ,$(1)))))
get-opt-memcpy = $(lastword $(subst _, ,$(filter opt-memcpy_%,$(subst __, ,$(1)))))
get-opt-pod-address = $(lastword $(subst _, ,$(filter opt-pod-address_%,$(subst __, ,$(1)))))
get-tiles-x = $(lastword $(subst _, ,$(filter tiles-x_%,$(subst __, ,$(1)))))
get-tiles-y = $(lastword $(subst _, ,$(filter tiles-y_%,$(subst __, ,$(1)))))
get-pods-x = $(lastword $(subst _, ,$(filter pods-x_%,$(subst __, ,$(1)))))
get-pods-y = $(lastword $(subst _, ,$(filter pods-y_%,$(subst __, ,$(1)))))
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
	@echo n = $(call get-n,$*) >> $@
	@echo task-size = $(call get-task-size,$*) >> $@
	@echo opt-memcpy = $(call get-opt-memcpy,$*) >> $@
	@echo opt-pod-address = $(call get-opt-pod-address,$*) >> $@
	@echo tiles-x = $(call get-tiles-x,$*) >> $@
	@echo tiles-y = $(call get-tiles-y,$*) >> $@
	@echo pods-x = $(call get-pods-x,$*) >> $@
	@echo pods-y = $(call get-pods-y,$*) >> $@
	@echo run = $(call get-run,$*) >> $@

include $(HB_HAMMERBENCH_PATH)/mk/testbench_common.mk


