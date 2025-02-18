HB_HAMMERBENCH_PATH = $(shell git rev-parse --show-toplevel)

.PHONY: all
all: generate

# call to generate a test name
test-name = push-front_$(1)__push-back_$(2)__pop-front_$(3)__pop-back_$(4)

# call to get parameter from test name
get-push-front = $(lastword $(subst _, ,$(filter push-front_%,$(subst __, ,$(1)))))
get-push-back = $(lastword $(subst _, ,$(filter push-back_%,$(subst __, ,$(1)))))
get-pop-front = $(lastword $(subst _, ,$(filter pop-front_%,$(subst __, ,$(1)))))
get-pop-back = $(lastword $(subst _, ,$(filter pop-back_%,$(subst __, ,$(1)))))

# defines tests
TESTS =
include tests.mk

TESTS_DIRS = $(TESTS)

$(addsuffix /parameters.mk,$(TESTS_DIRS)): %/parameters.mk:
	@echo Creating $@
	@mkdir -p $(dir $@)
	@touch $@
	@echo test-name  = $* >> $@
	@echo push-front = $(call get-push-front,$*) >> $@
	@echo push-back = $(call get-push-back,$*) >> $@
	@echo pop-front = $(call get-pop-front,$*) >> $@
	@echo pop-back = $(call get-pop-back,$*) >> $@

include $(HB_HAMMERBENCH_PATH)/mk/testbench_common.mk
