HB_HAMMERBENCH_PATH = $(shell git rev-parse --show-toplevel)

.PHONY: all
all: generate

# call to generate a test name
test-name = mtx_$(1)__tiles-x_$(2)__tiles-y_$(3)__pods-x_$(4)__pods-y_$(5)__opt-memcpy_$(6)__opt-restrict-ws_$(7)__opt-lock_$(8)__opt-rng_$(9)__opt-icache_$(10)__run_$(11)

# call to get parameter from test name
get-mtx = $(lastword $(subst _, ,$(filter mtx_%,$(subst __, ,$(1)))))
get-tiles-x = $(lastword $(subst _, ,$(filter tiles-x_%,$(subst __, ,$(1)))))
get-tiles-y = $(lastword $(subst _, ,$(filter tiles-y_%,$(subst __, ,$(1)))))
get-pods-x = $(lastword $(subst _, ,$(filter pods-x_%,$(subst __, ,$(1)))))
get-pods-y = $(lastword $(subst _, ,$(filter pods-y_%,$(subst __, ,$(1)))))
get-opt-memcpy = $(lastword $(subst _, ,$(filter opt-memcpy_%,$(subst __, ,$(1)))))
get-opt-restrict-ws = $(lastword $(subst _, ,$(filter opt-restrict-ws_%,$(subst __, ,$(1)))))
get-opt-lock = $(lastword $(subst _, ,$(filter opt-lock_%,$(subst __, ,$(1)))))
get-opt-rng = $(lastword $(subst _, ,$(filter opt-rng_%,$(subst __, ,$(1)))))
get-opt-icache = $(lastword $(subst _, ,$(filter opt-icache_%,$(subst __, ,$(1)))))
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
	@echo mtx = $(call get-mtx,$*) >> $@
	@echo tiles-x = $(call get-tiles-x,$*) >> $@
	@echo tiles-y = $(call get-tiles-y,$*) >> $@
	@echo pods-x = $(call get-pods-x,$*) >> $@
	@echo pods-y = $(call get-pods-y,$*) >> $@
	@echo opt-memcpy = $(call get-opt-memcpy,$*) >> $@
	@echo opt-restrict-ws = $(call get-opt-restrict-ws,$*) >> $@
	@echo opt-lock = $(call get-opt-lock,$*) >> $@
	@echo opt-rng = $(call get-opt-rng,$*) >> $@
	@echo opt-icache = $(call get-opt-icache,$*) >> $@
	@echo run = $(call get-run,$*) >> $@

include $(HB_HAMMERBENCH_PATH)/mk/testbench_common.mk


