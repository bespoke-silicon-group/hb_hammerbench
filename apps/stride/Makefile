HB_HAMMERBENCH_PATH = $(shell git rev-parse --show-toplevel)

.PHONY: all
all: generate

# call to generate a test name
test-name = loads_$(1)__stride-size_$(2)__ilp_$(3)

# call to get parameter from test name
get-loads = $(lastword $(subst _, ,$(filter loads_%,$(subst __, ,$(1)))))
get-stride-size = $(lastword $(subst _, ,$(filter stride-size_%,$(subst __, ,$(1)))))
get-ilp = $(lastword $(subst _, ,$(filter ilp_%,$(subst __, ,$(1)))))

# defines tests
TESTS =
include tests.mk

TESTS_DIRS = $(TESTS)

$(addsuffix /parameters.mk,$(TESTS_DIRS)): %/parameters.mk:
	@echo Creating $@
	@mkdir -p $(dir $@)
	@touch $@
	@echo test-name  = $* >> $@
	@echo loads = $(call get-loads,$*) >> $@
	@echo stride-size = $(call get-stride-size,$*) >> $@
	@echo ilp = $(call get-ilp,$*) >> $@

include $(HB_HAMMERBENCH_PATH)/mk/testbench_common.mk
