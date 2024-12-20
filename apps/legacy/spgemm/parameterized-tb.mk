##################################################################################
# BSD 3-Clause License								 #
# 										 #
# Copyright (c) 2022, Bespoke Silicon Group					 #
# All rights reserved.								 #
# 										 #
# Redistribution and use in source and binary forms, with or without		 #
# modification, are permitted provided that the following conditions are met:	 #
# 										 #
# 1. Redistributions of source code must retain the above copyright notice, this #
#    list of conditions and the following disclaimer.				 #
# 										 #
# 2. Redistributions in binary form must reproduce the above copyright notice,	 #
#    this list of conditions and the following disclaimer in the documentation	 #
#    and/or other materials provided with the distribution.			 #
# 										 #
# 3. Neither the name of the copyright holder nor the names of its		 #
#    contributors may be used to endorse or promote products derived from	 #
#    this software without specific prior written permission.			 #
# 										 #
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"	 #
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE	 #
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE #
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE	 #
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL	 #
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR	 #
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER	 #
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,	 #
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE	 #
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.		 #
##################################################################################
HB_HAMMERBENCH_PATH:=$(shell git rev-parse --show-toplevel)
include $(HB_HAMMERBENCH_PATH)/mk/environment.mk

# Set this to the path of your application.
# example: $(EXAMPLES_PATH)/cuda/dwarfs/spmv
ifndef APPLICATION_PATH
$(error "'APPLICATION_PATH' not defined; please define this variable to the absolute path to your application's directory")
endif

# Applications should define this function hook to add
# app specific parameters.
#
# 1: test-name
# 2: parameters.mk file name
#
# $(1) is the raw test name as found in the TESTS variable.
# $(2) is set to the parameters.mk of the test directory
# typically this is $(APPLICATION_PATH)/$(test-name)/parameters.mk
#
# example:
#
# define parameters-mk-add-application-params
# $(eval $(call get-test-parameters,$1)) # parse the test parameters from the test name
# @echo PARAM_0=$(PARAM_0) >> $2 # add first parameter
# @echo PARAM_1=$(PARAM_1) >> $2 # add second parameter
# endef

ifndef parameters-mk-add-application-params
$(warning "'parameters-mk-add-application-params' not defined; please define this function to populate parameters makefile.")
endif

# This is a list of parameterized 'tests' to run.
# A run directory will be generated for each element in this list.
# The parameters of the test should be extractable from the test name.
# This can be done by embedding the parameters in the test name (recommended).
# You can see bsg_replicant/examples/cuda/dwarfs/spmv for an example.
#
# Requirements:
# 
# The following character set is reserved in a test name and should not be used:
# [.]
ifndef TESTS
$(warning "'TESTS' variable is empty. Populate this variable with a list of tests to run. Running with no tests...")
endif

# This can be overriden to set a custom simulation directory for a test.
# Defaults to $(APPLICATION_PATH)/$(test-name)
#
# See spmm for examples of overriding
ifndef get-sim-dir-from-test
define get-sim-dir-from-test
$(eval SIMULATION_DIR=$(APPLICATION_PATH)/$1)
endef
endif

TESTS_PARAMETERS_MK=$(addsuffix /parameters.mk,$(TESTS))
TESTS_MAKEFILE=$(addsuffix /Makefile,$(TESTS))

# template.mk should exist in your application's directory.
# It should include `parameters.mk` and interface with the general bsg_replicant flow.
# This file is used to create $(APPLICATION_PATH)/$(test-name)/Makefile
#
# See bsg_replicant/examples/cuda/dwarf/spmv/template.mk for an example.
$(TESTS_MAKEFILE): %/Makefile: template.mk
	@echo "Creating $@"
	@mkdir -p $*
	@cp $< $@

$(TESTS_PARAMETERS_MK): %/parameters.mk:
	@echo "Creating $@"
	@mkdir -p $*
	@$(call get-sim-dir-from-test,$*)
	@echo APPLICATION_PATH=$(APPLICATION_PATH) > $@
	@echo SIMULATION_DIR=$(SIMULATION_DIR) >> $@
	@echo BSG_MANYCORE_KERNELS=$(SIMULATION_DIR)/kernel.riscv >> $@
	@$(call parameters-mk-add-application-params,$*,$@)

$(TESTS): %: %/Makefile %/parameters.mk

TESTS_EXEC    = $(addsuffix .exec,$(TESTS))
TESTS_DEBUG   = $(addsuffix .debug,$(TESTS))
TESTS_PROFILE = $(addsuffix .profile,$(TESTS))
TESTS_SAIFGEN = $(addsuffix .saifgen,$(TESTS))

$(TESTS_EXEC):    %.exec:    %
$(TESTS_DEBUG):   %.debug:   %
$(TESTS_PROFILE): %.profile: %
$(TESTS_SAIFGEN): %.saifgen: %

$(TESTS_EXEC) $(TESTS_DEBUG) $(TESTS_PROFILE) $(TESTS_SAIFGEN):
	@$(eval TEST_NAME=$(firstword $(subst ., ,$@)))
	@$(eval RUN_TYPE=$(lastword $(subst ., ,$@)))
	@echo "Running '$(RUN_TYPE)' on test $(TEST_NAME)"
	@$(MAKE) -C $(TEST_NAME) $(RUN_TYPE).log

exec:    $(TESTS_EXEC)
debug:   $(TEST_DEBUG)
saifgen: $(TESTS_SAIFGEN)
profile: $(TESTS_PROFILE)
tests:   $(TESTS)

purge:
	rm -rf $(TESTS)
