# HB_HAMMERBENCH_PATH = $(shell git rev-parse --show-toplevel)
HB_HAMMERBENCH_PATH := $(realpath $(dir $(lastword $(MAKEFILE_LIST)))/../..)

.PHONY: all
all: generate

# call to generate a test name
test-name = data-size_$(1)__warm-cache_$(2)

# call to get parameter from test name
get-data-size  = $(lastword $(subst _, ,$(filter data-size_%,$(subst __, ,$(1)))))
get-warm-cache = $(lastword $(subst _, ,$(filter warm-cache_%,$(subst __, ,$(1)))))

# defines tests
TESTS =
include tests.mk

TESTS_DIRS = $(TESTS)

$(addsuffix /parameters.mk,$(TESTS_DIRS)): %/parameters.mk:
	@echo Creating $@
	@mkdir -p $(dir $@)
	@touch $@
	@echo test-name  = $* >> $@
	@echo data-size = $(call get-data-size,$*) >> $@
	@echo warm-cache = $(call get-warm-cache,$*) >> $@

include $(HB_HAMMERBENCH_PATH)/mk/testbench_common.mk
