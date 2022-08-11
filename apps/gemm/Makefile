HB_HAMMERBENCH_PATH:=$(shell git rev-parse --show-toplevel)
include $(HB_HAMMERBENCH_PATH)/mk/environment.mk

########################
# Setup this directory #
########################
setup:
	@echo Nothing to be done

##############################
# CHANGE ME: IMPLEMENTATIONS #
##############################
IMPLS += sgemm_data_parallel
IMPLS += sgemm_group_cooperative

EXECUTION_DIRS := $(foreach impl,$(IMPLS),$(HB_HAMMERBENCH_PATH)/apps/gemm/$(impl))

include $(HB_HAMMERBENCH_PATH)/apps/common_complex.mk
