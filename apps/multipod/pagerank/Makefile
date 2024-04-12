HB_HAMMERBENCH_PATH = $(shell git rev-parse --show-toplevel)

.PHONY: all
all: generate

# test name
test-name = graph_$(1)__pod-id_$(2)__tgx_$(3)__tgy_$(4)


# get parameters;
get-graph  = $(lastword $(subst _, ,$(filter graph_%,$(subst __, ,$(1)))))
get-pod-id = $(lastword $(subst _, ,$(filter pod-id_%,$(subst __, ,$(1)))))
get-tgx = $(lastword $(subst _, ,$(filter tgx_%,$(subst __, ,$(1)))))
get-tgy = $(lastword $(subst _, ,$(filter tgy_%,$(subst __, ,$(1)))))

# tests;
TESTS =
include tests.mk

TESTS_DIRS = $(TESTS)


$(addsuffix /parameters.mk,$(TESTS_DIRS)): %/parameters.mk:
	@echo Creating $@
	@mkdir -p $(dir $@)
	@touch $@
	@echo test-name  = $* >> $@
	@echo graph  = $(call get-graph,$*) >> $@
	@echo pod-id = $(call get-pod-id,$*) >> $@
	@echo tgx = $(call get-tgx,$*) >> $@
	@echo tgy = $(call get-tgy,$*) >> $@


include $(HB_HAMMERBENCH_PATH)/mk/testbench_common.mk


clean:
	rm -rf graph_*pod-id*

