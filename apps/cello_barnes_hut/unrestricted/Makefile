HB_HAMMERBENCH_PATH = $(shell git rev-parse --show-toplevel)

.PHONY: all
all: generate

# call to generate a test name
test-name = nbodies_$(1)__tiles-x_$(2)__tiles-y_$(3)__pods-x_$(4)__pods-y_$(5)__unrestricted_$(6)__run_$(7)

# call to get parameter from test name
get-nbodies = $(lastword $(subst _, ,$(filter nbodies_%,$(subst __, ,$(1)))))
get-tiles-x = $(lastword $(subst _, ,$(filter tiles-x_%,$(subst __, ,$(1)))))
get-tiles-y = $(lastword $(subst _, ,$(filter tiles-y_%,$(subst __, ,$(1)))))
get-pods-x = $(lastword $(subst _, ,$(filter pods-x_%,$(subst __, ,$(1)))))
get-pods-y = $(lastword $(subst _, ,$(filter pods-y_%,$(subst __, ,$(1)))))
get-unrestricted = $(lastword $(subst _, ,$(filter unrestricted_%,$(subst __, ,$(1)))))
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
	@echo nbodies = $(call get-nbodies,$*) >> $@
	@echo tiles-x = $(call get-tiles-x,$*) >> $@
	@echo tiles-y = $(call get-tiles-y,$*) >> $@
	@echo pods-x = $(call get-pods-x,$*) >> $@
	@echo pods-y = $(call get-pods-y,$*) >> $@
	@echo unrestricted = $(call get-unrestricted,$*) >> $@
	@echo run = $(call get-run,$*) >> $@

include $(HB_HAMMERBENCH_PATH)/mk/testbench_common.mk


