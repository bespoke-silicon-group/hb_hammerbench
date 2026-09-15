HB_HAMMERBENCH_PATH:=$(shell git rev-parse --show-toplevel)
include $(HB_HAMMERBENCH_PATH)/mk/environment.mk

.PHONY: setup
.PHONY: generate
.DEFAULT_GOAL := generate
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
APPS += bfs
APPS += blackscholes
APPS += fft/128
APPS += fft/256
APPS += sgemm
APPS += jacobi
APPS += pagerank
APPS += smithwaterman
APPS += spgemm
APPS += memcpy
APPS += vector_add

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

APPS_rule_generate := $(foreach app,$(APPS),$(app).generate)
generate: $(APPS_rule_generate)
APPS_all_rules += $(APPS_rule_generate)

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
	$(eval requested_macro=$(lastword $(subst ., ,$@)))
	$(eval macro=$(if $(filter setup,$(requested_macro)),generate,$(requested_macro)))
	$(eval app_dir=$(call app-dir,$(app)))
	@echo Running $(macro) for $(app)
	@$(MAKE) -C $(app_dir) $(macro)
