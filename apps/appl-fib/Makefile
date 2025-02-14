HB_HAMMERBENCH_PATH = $(shell git rev-parse --show-toplevel)

.PHONY: all
all: generate

# call to generate a test name
test-name = fib-in_$(1)__grain-size_$(2)__tgx_$(3)__tgy_$(4)__appl-impl_$(5)

# call to get parameter from test name
get-fib-in = $(lastword $(subst _, ,$(filter fib-in_%,$(subst __, ,$(1)))))
get-grain-size = $(lastword $(subst _, ,$(filter grain-size_%,$(subst __, ,$(1)))))
get-tgx = $(lastword $(subst _, ,$(filter tgx_%,$(subst __, ,$(1)))))
get-tgy = $(lastword $(subst _, ,$(filter tgy_%,$(subst __, ,$(1)))))
get-appl-impl = $(lastword $(subst _, ,$(filter appl-impl_%,$(subst __, ,$(1)))))

# defines tests
TESTS =
include tests.mk

TESTS_DIRS = $(TESTS)

$(addsuffix /parameters.mk,$(TESTS_DIRS)): %/parameters.mk:
	@echo Creating $@
	@mkdir -p $(dir $@)
	@touch $@
	@echo test-name  = $* >> $@
	@echo fib-in = $(call get-fib-in,$*) >> $@
	@echo grain-size = $(call get-grain-size,$*) >> $@
	@echo tgx = $(call get-tgx,$*) >> $@
	@echo tgy = $(call get-tgy,$*) >> $@
	@echo appl-impl = $(call get-appl-impl,$*) >> $@

include $(HB_HAMMERBENCH_PATH)/mk/testbench_common.mk
