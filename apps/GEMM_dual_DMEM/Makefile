HB_HAMMERBENCH_PATH = $(shell git rev-parse --show-toplevel)

.PHONY: all
all: generate

# test name
# N_<N>__TGX_<tile_group_x>__TGY_<tile_group_y>
test-name = N_$(1)__TGX_$(2)__TGY_$(3)

# parse parameters from test name
get-N   = $(lastword $(subst _, ,$(filter N_%,$(subst __, ,$(1)))))
get-TGX = $(lastword $(subst _, ,$(filter TGX_%,$(subst __, ,$(1)))))
get-TGY = $(lastword $(subst _, ,$(filter TGY_%,$(subst __, ,$(1)))))

# tests
TESTS =
include tests.mk

TESTS_DIRS = $(TESTS)

$(addsuffix /parameters.mk,$(TESTS_DIRS)): %/parameters.mk:
	@echo Creating $@
	@mkdir -p $(dir $@)
	@touch $@
	@echo test-name = $* >> $@
	@echo N = $(call get-N,$*) >> $@
	@echo tgx = $(call get-TGX,$*) >> $@
	@echo tgy = $(call get-TGY,$*) >> $@

include $(HB_HAMMERBENCH_PATH)/mk/testbench_common.mk

clean:
	rm -rf N_*__TGX_*__TGY_*
