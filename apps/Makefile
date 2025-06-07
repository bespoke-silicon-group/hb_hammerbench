HB_HAMMERBENCH_PATH:=$(shell git rev-parse --show-toplevel)
include $(HB_HAMMERBENCH_PATH)/mk/environment.mk

.PHONY: setup
.PHONY: profile
.PHONY: exec
.PHONY: pc-histogram
.PHONY: debug
.PHONY: repl
.PHONY: saifgen
.PHONY: stats
.PHONY: clean

##############################
# CHANGE ME: APPLICATIONS    #
# Add to or remove from apps #
##############################
APPS := aes
APPS += barnes_hut
# APPS += bfs
APPS += blackscholes
APPS += fft
APPS += sgemm_group_cooperative
APPS += jacobi
# APPS += pagerank
APPS += smith_waterman
APPS += spgemm
APPS += memcpy
APPS += gups_rmw

###########################
# directories of each app #
###########################
app-dir = $(HB_HAMMERBENCH_PATH)/apps/$1
APPS_dirs  := $(foreach app,$(APPS),$(call app-dir,$(app)))

#########################################
# setup rules to be run on fresh clone  #
#########################################
APPS_rule_setup      := $(foreach app,$(APPS),$(app).setup)
setup: $(APPS_rule_setup)
APPS_all_rules += $(APPS_rule_setup)

#########################################################
# for each macro rule				        #
# 	1. create list of rules for that macro	        #
# 	2. set dependencies for that macro	        #
# 	3. add to list of all rules		        #
#########################################################
APPS_rule_profile      := $(foreach app,$(APPS),$(app).profile)
profile: $(APPS_rule_profile)
APPS_all_rules += $(APPS_rule_profile)

APPS_rule_exec         := $(foreach app,$(APPS),$(app).exec)
exec: $(APPS_rule_exec)
APPS_all_rules += $(APPS_rule_exec)

APPS_rule_pc-histogram := $(foreach app,$(APPS),$(app).pc-histogram)
pc-histogram: $(APPS_rule_pc-histogram)
APPS_all_rules += $(APPS_rule_pc-histogram)

APPS_rule_debug        := $(foreach app,$(APPS),$(app).debug)
debug: $(APPS_rule_debug)
APPS_all_rules += $(APPS_rule_debug)

APPS_rule_repl         := $(foreach app,$(APPS),$(app).repl)
repl: $(APPS_rule_repl)
APPS_all_rules += $(APPS_rule_repl)

APPS_rule_saifgen      := $(foreach app,$(APPS),$(app).saifgen)
saifgen: $(APPS_rule_saifgen)
APPS_all_rules += $(APPS_rule_saifgen)

APPS_rule_stats        := $(foreach app,$(APPS),$(app).stats)
stats: $(APPS_rule_stats)
APPS_all_rules += $(APPS_rule_stats)

APPS_rule_clean        := $(foreach app,$(APPS),$(app).clean)
clean: $(APPS_rule_clean)
APPS_all_rules += $(APPS_rule_clean)

$(APPS_all_rules):
	$(eval app=$(firstword $(subst ., ,$@)))
	$(eval macro=$(lastword  $(subst ., ,$@)))
	$(eval app_dir=$(call app-dir,$(app)))
	@echo Running $(macro) for $(app)
	@$(MAKE) -C $(app_dir) $(macro)




RECURSE_APPS = memcpy vector_add-yes vector_add-no sgemm aes jacobi-yes jacobi-no blackscholes smithwaterman fft_128m-yes fft_128m-no
RECURSE_CLEANS = memcpy vector_add sgemm aes jacobi blackscholes smithwaterman fft_128m

recurse-all: $(foreach n, $(RECURSE_APPS), recurse-$(n))
recurse-memcpy:
	$(MAKE) -C memcpy generate
	$(MAKE) -C memcpy/tile-x_4__tile-y_4__buffer-size_4096__warm-cache_no exec.log
recurse-vector_add-yes:
	$(MAKE) -C vector_add generate
	$(MAKE) -C vector_add/vector-size_65536__warm-cache_yes exec.log
recurse-vector_add-no:
	sleep 10
	$(MAKE) -C vector_add generate
	$(MAKE) -C vector_add/vector-size_65536__warm-cache_no exec.log
recurse-sgemm:
	$(MAKE) -C sgemm generate
	$(MAKE) -C sgemm/N_64__NITER_2 exec.log
recurse-aes:
	$(MAKE) -C aes generate
	$(MAKE) -C aes/NUM-ITER_2 exec.log
recurse-jacobi-yes:
	$(MAKE) -C jacobi generate
	$(MAKE) -C jacobi/nx_32__ny_8__nz_512__warm-cache_yes exec.log
recurse-jacobi-no:
	sleep 10
	$(MAKE) -C jacobi generate
	$(MAKE) -C jacobi/nx_64__ny_8__nz_512__warm-cache_no exec.log
recurse-blackscholes:
	$(MAKE) -C blackscholes generate
	$(MAKE) -C blackscholes/num-option_32768 exec.log
recurse-smithwaterman:
	$(MAKE) -C smithwaterman generate
	$(MAKE) -C smithwaterman/num-seq_512 exec.log
recurse-fft_128m-yes:
	$(MAKE) -C fft_128m generate
	$(MAKE) -C fft_128m/num-iter_1__warm-cache_yes exec.log
recurse-fft_128m-no:
	sleep 10
	$(MAKE) -C fft_128m generate
	$(MAKE) -C fft_128m/num-iter_1__warm-cache_no exec.log

recurse-clean: $(foreach n, $(RECURSE_CLEANS), recurse-clean-$(n))
recurse-clean-%:
	$(MAKE) -C $* purge

recurse-summary:
	@grep --color -r PASSED   */*/exec.log || true
	@grep --color -r PASSED */*/*/exec.log || true
	@grep --color -r FAILED   */*/exec.log || true
	@grep --color -r FAILED */*/*/exec.log || true
