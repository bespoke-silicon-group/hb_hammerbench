# This Makefile compiles, links, and executes examples Run `make help`
# to see the available targets for the selected platform.

################################################################################
# environment.mk verifies the build environment and sets the following
# makefile variables:
#
# LIBRAIRES_PATH: The path to the libraries directory
# HARDWARE_PATH: The path to the hardware directory
# EXAMPLES_PATH: The path to the examples directory
# BASEJUMP_STL_DIR: Path to a clone of BaseJump STL
# BSG_MANYCORE_DIR: Path to a clone of BSG Manycore
###############################################################################
HB_HAMMERBENCH_PATH:=$(shell git rev-parse --show-toplevel)
REPLICANT_PATH := $(shell cd $(HB_HAMMERBENCH_PATH)/.. && git rev-parse --show-toplevel)
#BSG_MACHINE_PATH := $(REPLICANT_PATH)/machines/pod_X1Y1_ruche_X4Y2_hbm_one_pseudo_channel
#BSG_MACHINE_PATH := $(REPLICANT_PATH)/machines/pod_X2Y2_ruche_X4Y2_hbm

include $(HB_HAMMERBENCH_PATH)/mk/environment.mk
include $(HB_HAMMERBENCH_PATH)/mk/cello.mk

###############################################################################
# Host code compilation flags and flow
###############################################################################
# import parameters and APP_PATH
include parameters.mk
include app_path.mk

PODS_X := $(pods-x)
PODS_Y := $(pods-y)

# Tile Group Dimensions
TILE_GROUP_DIM_X := $(tiles-x)
TILE_GROUP_DIM_Y := $(tiles-y)

vpath %.c   $(APP_PATH)
vpath %.cpp $(APP_PATH)

# TEST_SOURCES is a list of source files that need to be compiled
TEST_SOURCES += host.cpp

DEFINES += -D_XOPEN_SOURCE=500 -D_BSD_SOURCE -D_DEFAULT_SOURCE -DNUM_SEQ=$(num-seq)
CDEFINES += -Dbsg_tiles_X=$(TILE_GROUP_DIM_X) -Dbsg_tiles_Y=$(TILE_GROUP_DIM_Y)
CXXDEFINES +=

FLAGS     = -g -Wall -Wno-unused-function -Wno-unused-variable
CFLAGS   += -std=c99 $(FLAGS)
CXXFLAGS += -std=c++11 $(FLAGS)

# compilation.mk defines rules for compilation of C/C++
include $(EXAMPLES_PATH)/compilation.mk

###############################################################################
# Host code link flags and flow
###############################################################################

LDFLAGS +=

# link.mk defines rules for linking of the final execution binary.
include $(EXAMPLES_PATH)/link.mk

###############################################################################
# Device code compilation flow
###############################################################################

# BSG_MANYCORE_KERNELS is a list of manycore executables that should
# be built before executing.

# RISCV_CCPPFLAGS += -DTRACE # uncomment to enable trace
RISCV_CCPPFLAGS += -DNUM_SEQ=$(num-seq)
RISCV_CCPPFLAGS += -O3 -std=c++14
RISCV_CCPPFLAGS += -Dbsg_tiles_X=$(TILE_GROUP_DIM_X)
RISCV_CCPPFLAGS += -Dbsg_tiles_Y=$(TILE_GROUP_DIM_Y)
ifeq ($(opt-memcpy),yes)
RISCV_CCPPFLAGS += -DBSG_GLOBAL_POINTER_OPT_MEMCPY
endif
ifeq ($(opt-resrict-ws),no)
RISCV_CCPPFLAGS += -DCELLO_GLOBAL_STEALING
endif
ifeq ($(opt-lock),no)
RISCV_CCPPFLAGS += -DCELLO_THIEF_LOCK
RISCV_CCPPFLAGS += -DUTIL_LOCK_NO_EXPONENTIAL_BACKOFF
endif
ifeq ($(opt-rng),yes)
RISCV_CCPPFLAGS += -DCELLO_FAST_RANDOM_XORSHIFT
endif
RISCV_TARGET_OBJECTS += kernel.rvo
BSG_MANYCORE_KERNELS := main.riscv

include $(EXAMPLES_PATH)/cuda/riscv.mk
###############################################################################
# Execution flow
#
# C_ARGS: Use this to pass arguments that you want to appear in argv
#         For SPMD tests C arguments are: <Path to RISC-V Binary> <Test Name>
#
# SIM_ARGS: Use this to pass arguments to the simulator
###############################################################################
C_ARGS ?= $(BSG_MANYCORE_KERNELS)
C_ARGS += $(APP_PATH)/dna-query32.fasta
C_ARGS += $(APP_PATH)/dna-reference32.fasta
C_ARGS += $(APP_PATH)/output32

SIM_ARGS ?=

# Include platform-specific execution rules
include $(EXAMPLES_PATH)/execution.mk

###############################################################################
# Regression Flow
###############################################################################

regression: exec.log
	@grep "BSG REGRESSION TEST .*PASSED.*" $< > /dev/null

.DEFAULT_GOAL := help


