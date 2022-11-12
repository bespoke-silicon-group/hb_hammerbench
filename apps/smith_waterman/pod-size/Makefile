HB_HAMMERBENCH_PATH = $(shell git rev-parse --show-toplevel)

.PHONY: all
all: generate

# call to generate a test name
test-name = tiles-x_$(1)__tiles-y_$(2)

# call to get parameter from test name
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
	@echo tiles-x = $(call get-tiles-x,$*) >> $@
	@echo tiles-y = $(call get-tiles-y,$*) >> $@

include $(HB_HAMMERBENCH_PATH)/mk/testbench_common.mk

# # for regression
# EXECUTION_DIRS := (addprefix $(APP_PATH)/,$(TESTS_DIRS))
# include $(HB_HAMMERBENCH_PATH)/apps/common_complex.mk


