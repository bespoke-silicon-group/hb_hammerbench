HB_HAMMERBENCH_PATH = $(shell git rev-parse --show-toplevel)

.PHONY: all
all: generate

# call to generate a test name
test-name = mtx-a_$(1)__mtx-b_$(2)__tiles-x_$(3)__tiles-y_$(4)__pods-x_$(5)__pods-y_$(6)

# call to get parameter from test name
get-mtx-a = $(lastword $(subst _, ,$(filter mtx-a_%,$(subst __, ,$(1)))))
get-mtx-b = $(lastword $(subst _, ,$(filter mtx-b_%,$(subst __, ,$(1)))))
get-tiles-x = $(lastword $(subst _, ,$(filter tiles-x_%,$(subst __, ,$(1)))))
get-tiles-y = $(lastword $(subst _, ,$(filter tiles-y_%,$(subst __, ,$(1)))))
get-pods-x = $(lastword $(subst _, ,$(filter pods-x_%,$(subst __, ,$(1)))))
get-pods-y = $(lastword $(subst _, ,$(filter pods-y_%,$(subst __, ,$(1)))))

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
	@echo tiles-x = $(call get-tiles-x,$*) >> $@
	@echo tiles-y = $(call get-tiles-y,$*) >> $@
	@echo pods-x = $(call get-pods-x,$*) >> $@
	@echo pods-y = $(call get-pods-y,$*) >> $@

include $(HB_HAMMERBENCH_PATH)/mk/testbench_common.mk


