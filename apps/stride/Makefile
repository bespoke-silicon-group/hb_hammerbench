HB_HAMMERBENCH_PATH = $(shell git rev-parse --show-toplevel)

.PHONY: all
all: generate

# call to generate a test name
test-name = n_$(1)__stride-size_$(2)__vector-size_$(3)__warm-cache_$(4)__tile-x_$(5)__tile-y_$(6)__pod-x_$(7)__pod-y_$(8)

# call to get parameter from test name
get-n = $(lastword $(subst _, ,$(filter n_%,$(subst __, ,$(1)))))
get-stride-size = $(lastword $(subst _, ,$(filter stride-size_%,$(subst __, ,$(1)))))
get-vector-size = $(lastword $(subst _, ,$(filter vector-size_%,$(subst __, ,$(1)))))
get-warm-cache = $(lastword $(subst _, ,$(filter warm-cache_%,$(subst __, ,$(1)))))
get-tile-x = $(lastword $(subst _, ,$(filter tile-x_%,$(subst __, ,$(1)))))
get-tile-y = $(lastword $(subst _, ,$(filter tile-y_%,$(subst __, ,$(1)))))
get-pod-x = $(lastword $(subst _, ,$(filter pod-x_%,$(subst __, ,$(1)))))
get-pod-y = $(lastword $(subst _, ,$(filter pod-y_%,$(subst __, ,$(1)))))

# defines tests
TESTS =
include tests.mk

TESTS_DIRS = $(TESTS)

$(addsuffix /parameters.mk,$(TESTS_DIRS)): %/parameters.mk:
	@echo Creating $@
	@mkdir -p $(dir $@)
	@touch $@
	@echo test-name  = $* >> $@
	@echo n = $(call get-n,$*) >> $@
	@echo stride-size = $(call get-stride-size,$*) >> $@
	@echo vector-size = $(call get-vector-size,$*) >> $@
	@echo warm-cache = $(call get-warm-cache,$*) >> $@
	@echo tile-x = $(call get-tile-x,$*) >> $@
	@echo tile-y = $(call get-tile-y,$*) >> $@
	@echo pod-x = $(call get-pod-x,$*) >> $@
	@echo pod-y = $(call get-pod-y,$*) >> $@

include $(HB_HAMMERBENCH_PATH)/mk/testbench_common.mk

