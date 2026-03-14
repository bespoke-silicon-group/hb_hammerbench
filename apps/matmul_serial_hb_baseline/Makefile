HB_HAMMERBENCH_PATH = $(shell git rev-parse --show-toplevel)

.PHONY: all
all: generate

# test name
test-name = N_$(1)__NITER_$(2)__tile-x_$(3)__tile-y_$(4)__PATTERN_$(5)__warm-cache_$(6)


# Get parameters;
get-N     = $(lastword $(subst _, ,$(filter N_%,$(subst __, ,$(1)))))
get-NITER = $(lastword $(subst _, ,$(filter NITER_%,$(subst __, ,$(1)))))
get-tile-x = $(lastword $(subst _, ,$(filter tile-x_%,$(subst __, ,$(1)))))
get-tile-y = $(lastword $(subst _, ,$(filter tile-y_%,$(subst __, ,$(1)))))
get-PATTERN = $(lastword $(subst _, ,$(filter PATTERN_%,$(subst __, ,$(1)))))
get-warm-cache = $(lastword $(subst _, ,$(filter warm-cache_%,$(subst __, ,$(1)))))


# tests;
TESTS =
include tests.mk

TESTS_DIRS = $(TESTS)


$(addsuffix /parameters.mk,$(TESTS_DIRS)): %/parameters.mk:
	@echo Creating $@
	@mkdir -p $(dir $@)
	@touch $@
	@echo test-name  = $* >> $@
	@echo N     = $(call get-N,$*) >> $@
	@echo NITER = $(call get-NITER,$*) >> $@
	@echo tile-x = $(call get-tile-x,$*) >> $@
	@echo tile-y = $(call get-tile-y,$*) >> $@
	@echo PATTERN = $(call get-PATTERN,$*) >> $@
	@echo warm-cache = $(call get-warm-cache,$*) >> $@

include $(HB_HAMMERBENCH_PATH)/mk/testbench_common.mk

clean:
	rm -rf N_*NITER*
