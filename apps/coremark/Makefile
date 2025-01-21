HB_HAMMERBENCH_PATH = $(shell git rev-parse --show-toplevel)

.PHONY: all
all: generate

# test name
test-name = NITER_$(1)


# Get parameters;
get-NITER = $(lastword $(subst _, ,$(filter NITER_%,$(subst __, ,$(1)))))


# tests;
TESTS =
include tests.mk

TESTS_DIRS = $(TESTS)


$(addsuffix /parameters.mk,$(TESTS_DIRS)): %/parameters.mk:
	@echo Creating $@
	@mkdir -p $(dir $@)
	@touch $@
	@echo test-name  = $* >> $@
	@echo NITER = $(call get-NITER,$*) >> $@


include $(HB_HAMMERBENCH_PATH)/mk/testbench_common.mk

clean:
	rm -rf NITER*
