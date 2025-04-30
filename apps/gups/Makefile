HB_HAMMERBENCH_PATH = $(shell git rev-parse --show-toplevel)

.PHONY: all
all: generate

# call to generate a test name
test-name = updates_$(1)__size_$(2)

# call to get parameter from test name
get-updates = $(lastword $(subst _, ,$(filter updates_%,$(subst __, ,$(1)))))
get-size = $(lastword $(subst _, ,$(filter size_%,$(subst __, ,$(1)))))

# defines tests
TESTS =
include tests.mk

TESTS_DIRS = $(TESTS)

$(addsuffix /parameters.mk,$(TESTS_DIRS)): %/parameters.mk:
	@echo Creating $@
	@mkdir -p $(dir $@)
	@touch $@
	@echo test-name  = $* >> $@
	@echo updates = $(call get-updates,$*) >> $@
	@echo size = $(call get-size,$*) >> $@

include $(HB_HAMMERBENCH_PATH)/mk/testbench_common.mk
