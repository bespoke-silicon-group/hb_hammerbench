REPLICANT_PATH = $(shell git rev-parse --show-toplevel)

.PHONY: all
all: generate


# call to generate a test name
test-name = num-iter_$(1)__warm-cache_$(2)

# call to get parameter from test name
get-num-iter = $(lastword $(subst _, ,$(filter num-iter_%,$(subst __, ,$(1)))))
get-warm-cache = $(lastword $(subst _, ,$(filter warm-cache_%,$(subst __, ,$(1)))))

# defines tests
TESTS = 
include tests.mk

TESTS_DIRS = $(TESTS)

$(addsuffix /parameters.mk,$(TESTS_DIRS)): %/parameters.mk:
	@echo Creating $@
	@mkdir -p $(dir $@)
	@touch $@
	@echo test-name  = $* >> $@
	@echo num-iter = $(call get-num-iter,$*) >> $@
	@echo warm-cache = $(call get-warm-cache,$*) >> $@

$(addsuffix /app_path.mk,$(TESTS_DIRS)): %/app_path.mk: app_path.mk
	@echo Creating $@
	@mkdir -p $(dir $@)
	@cp $< $@

$(addsuffix /Makefile,$(TESTS_DIRS)): %/Makefile: template.mk
	@echo Creating $@
	@mkdir -p $(dir $@)
	@cp template.mk $@

$(TESTS_DIRS): %: %/app_path.mk
$(TESTS_DIRS): %: %/parameters.mk
$(TESTS_DIRS): %: %/Makefile

generate: $(TESTS_DIRS)

$(addsuffix .profile,$(TESTS)): %.profile: %
	$(MAKE) -C $< profile.log

$(addsuffix .pc-histogram,$(TESTS)): %.pc-histogram: %
	$(MAKE) -C $< pc-histogram.log

$(addsuffix .exec,$(TESTS)): %.exec: %
	$(MAKE) -C $< exec.log

$(addsuffix .stats,$(TESTS)): %.stats: %
	$(MAKE) -C $< stats

$(addsuffix .pchistpdf,$(TESTS)): %.pchistpdf: %
	$(MAKE) -C $< pchistpdf

.PHONY: profile exec

pc-histogram: $(addsuffix .pc-histogram,$(TESTS))
profile: $(addsuffix .profile,$(TESTS))
exec:    $(addsuffix .exec,$(TESTS))
stats:   $(addsuffix .stats,$(TESTS))
pchistpdf: $(addsuffix .pchistpdf,$(TESTS))
purge:
	rm -rf $(TESTS_DIRS)
