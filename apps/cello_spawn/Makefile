HB_HAMMERBENCH_PATH = $(shell git rev-parse --show-toplevel)

.PHONY: all
all: generate

# call to generate a test name
test-name = n_$(1)__size_$(2)__dmem_$(3)__one-child-joiner_$(4)__no-spawn_$(5)__tiles-x_$(6)__tiles-y_$(7)__pods-x_$(8)__pods-y_$(9)__run_$(10)

# call to get parameter from test name
get-n = $(lastword $(subst _, ,$(filter n_%,$(subst __, ,$(1)))))
get-size = $(lastword $(subst _, ,$(filter size_%,$(subst __, ,$(1)))))
get-dmem = $(lastword $(subst _, ,$(filter dmem_%,$(subst __, ,$(1)))))
get-one-child-joiner = $(lastword $(subst _, ,$(filter one-child-joiner_%,$(subst __, ,$(1)))))
get-no-spawn = $(lastword $(subst _, ,$(filter no-spawn_%,$(subst __, ,$(1)))))
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
	@echo size = $(call get-size,$*) >> $@
	@echo dmem = $(call get-dmem,$*) >> $@
	@echo one-child-joiner = $(call get-one-child-joiner,$*) >> $@
	@echo no-spawn = $(call get-no-spawn,$*) >> $@
	@echo tiles-x = $(call get-tiles-x,$*) >> $@
	@echo tiles-y = $(call get-tiles-y,$*) >> $@
	@echo pods-x = $(call get-pods-x,$*) >> $@
	@echo pods-y = $(call get-pods-y,$*) >> $@
	@echo run = $(call get-run,$*) >> $@

include $(HB_HAMMERBENCH_PATH)/mk/testbench_common.mk


