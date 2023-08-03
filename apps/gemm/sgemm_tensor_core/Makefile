HB_HAMMERBENCH_PATH = $(shell git rev-parse --show-toplevel)

.PHONY: all
all: generate


# call to generate a test name
test-name = tile-x_$(1)__tile-y_$(2)__N_$(3)__Z_$(4)

# call to get parameter from test name
get-tile-x= $(lastword $(subst _, ,$(filter tile-x_%,$(subst __, ,$(1)))))
get-tile-y= $(lastword $(subst _, ,$(filter tile-y_%,$(subst __, ,$(1)))))
get-N= $(lastword $(subst _, ,$(filter N_%,$(subst __, ,$(1)))))
get-Z= $(lastword $(subst _, ,$(filter Z_%,$(subst __, ,$(1)))))

# defines tests
TESTS =
include tests.mk

TESTS_DIRS = $(TESTS)

$(addsuffix /parameters.mk,$(TESTS_DIRS)): %/parameters.mk:
	@echo Creating $@
	@mkdir -p $(dir $@)
	@touch $@
	@echo test-name  = $* >> $@
	@echo tile-x  = $(call get-tile-x,$*) >> $@
	@echo tile-y  = $(call get-tile-y,$*) >> $@
	@echo N  = $(call get-N,$*) >> $@
	@echo Z  = $(call get-Z,$*) >> $@

include $(HB_HAMMERBENCH_PATH)/mk/testbench_common.mk

