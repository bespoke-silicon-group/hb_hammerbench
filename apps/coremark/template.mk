include parameters.mk
include app_path.mk

# Hardware;
HB_HAMMERBENCH_PATH:=$(shell git rev-parse --show-toplevel)

tile-x?=16
tile-y?=8
override BSG_MACHINE_PATH = $(REPLICANT_PATH)/machines/pod_X1Y1_ruche_X$(tile-x)Y$(tile-y)_hbm_one_pseudo_channel
include $(HB_HAMMERBENCH_PATH)/mk/environment.mk


# number of pods participating in barrier;
NUM_POD_X=$(BSG_MACHINE_PODS_X)
NUM_POD_Y=$(BSG_MACHINE_PODS_Y)
# Tile group DIM
TILE_GROUP_DIM_X ?= 16
TILE_GROUP_DIM_Y ?= 8

vpath %.c   $(APP_PATH)
vpath %.cpp $(APP_PATH)


# Test sources;
TEST_SOURCES := main.cpp

DEFINES += -D_XOPEN_SOURCE=500 -D_BSD_SOURCE -D_DEFAULT_SOURCE
DEFINES += -Dbsg_tiles_X=$(TILE_GROUP_DIM_X) -Dbsg_tiles_Y=$(TILE_GROUP_DIM_Y)
DEFINES += -DNUM_POD_X=$(NUM_POD_X) # number of pods simulating now;
DEFINES += -DNITER=$(NITER)

FLAGS     = -g -Wall -Wno-unused-function -Wno-unused-variable
CFLAGS   += -std=c99 $(FLAGS)
CXXFLAGS += -std=c++11 $(FLAGS)


# compilation rules;
include $(EXAMPLES_PATH)/compilation.mk

# Linker rules;
LDFLAGS +=
include $(EXAMPLES_PATH)/link.mk


# Device code;
vpath %.c $(BSG_MANYCORE_DIR)/imports/coremark

RISCV_CCPPFLAGS += -O3 -std=c++14
RISCV_CCPPFLAGS += -I$(HB_HAMMERBENCH_PATH)/apps/common
RISCV_CCPPFLAGS += -I$(HB_HAMMERBENCH_PATH)/apps/coremark
RISCV_CCPPFLAGS += -I$(BSG_MANYCORE_DIR)/imports/coremark
RISCV_CCPPFLAGS += -DNUM_POD_X=$(NUM_POD_X)
RISCV_CCPPFLAGS += -DBSG_MACHINE_GLOBAL_X=$(BSG_MACHINE_GLOBAL_X)
RISCV_CCPPFLAGS += -DBSG_MACHINE_GLOBAL_Y=$(BSG_MACHINE_GLOBAL_Y)
RISCV_CCPPFLAGS += -Dbsg_tiles_X=$(TILE_GROUP_DIM_X)
RISCV_CCPPFLAGS += -Dbsg_tiles_Y=$(TILE_GROUP_DIM_Y)
RISCV_CCPPFLAGS += -DITERATIONS=$(NITER)

RISCV_TARGET_OBJECTS = kernel.rvo core_list_join.rvo core_matrix.rvo core_state.rvo core_util.rvo core_portme.rvo
BSG_MANYCORE_KERNELS = main.riscv

include $(EXAMPLES_PATH)/cuda/riscv.mk


# Execution args;
C_ARGS ?= $(BSG_MANYCORE_KERNELS)

			
SIM_ARGS ?=  +vcs+nostdout


# Exec rules;
include $(EXAMPLES_PATH)/execution.mk
RUN_RULES += saifgen.log
RUN_RULES += repl.log
RUN_RULES += pc-histogram.log
RUN_RULES += debug.log
RUN_RULES += profile.log
RUN_RULES += exec.log
.DEFAULT_GOAL := help
