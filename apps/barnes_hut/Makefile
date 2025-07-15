HB_HAMMERBENCH_PATH = $(shell git rev-parse --show-toplevel)

.PHONY: all
all: generate

# call to generate a test name
test-name = nbodies_$(1)__pod-id_$(2)__run_$(3)

# call to get parameter from test name
get-nbodies = $(lastword $(subst _, ,$(filter nbodies_%,$(subst __, ,$(1)))))
get-pod-id = $(lastword $(subst _, ,$(filter pod-id_%,$(subst __, ,$(1)))))
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
	@echo pod-id = $(call get-pod-id,$*) >> $@
	@echo run = $(call get-run,$*) >> $@

include $(HB_HAMMERBENCH_PATH)/mk/testbench_common.mk


