HB_HAMMERBENCH_PATH = $(shell git rev-parse --show-toplevel)

.PHONY: all
all: generate

# call to generate a test name
test-name = input_$(1)__start_$(2)__tiles-x_$(3)__tiles-y_$(4)

# call to get parameter from test name
get-input = $(lastword $(subst _, ,$(filter input_%,$(subst __, ,$(1)))))
get-start = $(lastword $(subst _, ,$(filter start_%,$(subst __, ,$(1)))))
get-tiles-x = $(lastword $(subst _, ,$(filter tiles-x_%,$(subst __, ,$(1)))))
get-tiles-y = $(lastword $(subst _, ,$(filter tiles-y_%,$(subst __, ,$(1)))))

# defines tests
TESTS =
include tests.mk

TESTS_DIRS = $(TESTS)

$(addsuffix /parameters.mk,$(TESTS_DIRS)): %/parameters.mk:
	@echo Creating $@
	@mkdir -p $(dir $@)
	@touch $@
	@echo test-name  = $* >> $@
	@echo input = $(call get-input,$*) >> $@
	@echo start = $(call get-start,$*) >> $@
	@echo tiles-x = $(call get-tiles-x,$*) >> $@
	@echo tiles-y = $(call get-tiles-y,$*) >> $@

include $(HB_HAMMERBENCH_PATH)/mk/testbench_common.mk

# # for regression
# EXECUTION_DIRS := (addprefix $(APP_PATH)/,$(TESTS_DIRS))
# include $(HB_HAMMERBENCH_PATH)/apps/common_complex.mk


