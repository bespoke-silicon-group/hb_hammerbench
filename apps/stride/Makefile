HB_HAMMERBENCH_PATH = $(shell git rev-parse --show-toplevel)

.PHONY: all
all: generate

# call to generate a test name
test-name = stride-size_$(1)__stride-n_$(2)__x_$(3)__y_$(4)__warmup_$(5)__tiles-x_$(6)__tiles-y_$(7)

# call to get parameter from test name
get-stride-size = $(lastword $(subst _, ,$(filter stride-size_%,$(subst __, ,$(1)))))
get-stride-n = $(lastword $(subst _, ,$(filter stride-n_%,$(subst __, ,$(1)))))
get-x = $(lastword $(subst _, ,$(filter x_%,$(subst __, ,$(1)))))
get-y = $(lastword $(subst _, ,$(filter y_%,$(subst __, ,$(1)))))
get-warmup = $(lastword $(subst _, ,$(filter warmup_%,$(subst __, ,$(1)))))
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
	@echo stride-size = $(call get-stride-size,$*) >> $@
	@echo stride-n = $(call get-stride-n,$*) >> $@
	@echo x = $(call get-x,$*) >> $@
	@echo y = $(call get-y,$*) >> $@
	@echo warmup = $(call get-warmup,$*) >> $@
	@echo tiles-x = $(call get-tiles-x,$*) >> $@
	@echo tiles-y = $(call get-tiles-y,$*) >> $@

include $(HB_HAMMERBENCH_PATH)/mk/testbench_common.mk

# for regression
EXECUTION_DIRS := (addprefix $(APP_PATH)/,$(TESTS_DIRS))


