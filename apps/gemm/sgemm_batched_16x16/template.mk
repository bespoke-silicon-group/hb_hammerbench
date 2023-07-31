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
include parameters.mk
include app_path.mk
HB_HAMMERBENCH_PATH:=$(shell git rev-parse --show-toplevel)
override BSG_MACHINE_PATH = $(REPLICANT_PATH)/machines/pod_X1Y1_ruche_X$(tile-x)Y$(tile-y)_hbm_one_pseudo_channel
include $(HB_HAMMERBENCH_PATH)/mk/environment.mk

###############################################################################
# Host code compilation flags and flow
###############################################################################
# import parameters and APP_PATH

# Tile Group Dimensions
TILE_GROUP_DIM_X ?= $(tile-x)
TILE_GROUP_DIM_Y ?= $(tile-y)
BLOCK_DIM_X ?= 8
BLOCK_DIM_Y ?= 16
BATCH ?= 2048
UNFOLD ?= 1
WARM_CACHE ?= no

vpath %.c   $(APP_PATH)
vpath %.cpp $(APP_PATH)

TEST_SOURCES = main.cpp

DEFINES += -D_XOPEN_SOURCE=500 -D_BSD_SOURCE -D_DEFAULT_SOURCE
DEFINES += -Dbsg_tiles_X=$(TILE_GROUP_DIM_X)
DEFINES += -Dbsg_tiles_Y=$(TILE_GROUP_DIM_Y)
DEFINES += -DBLOCK_DIM_X=$(BLOCK_DIM_X)
DEFINES += -DBLOCK_DIM_Y=$(BLOCK_DIM_Y)
DEFINES += -DBATCH=$(BATCH)
CDEFINES += 
CXXDEFINES += 

FLAGS     = -g -Wall -Wno-unused-function -Wno-unused-variable
CFLAGS   += -std=gnu99 $(FLAGS)
CXXFLAGS += -std=c++11 $(FLAGS) -O3

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
BSG_MANYCORE_KERNELS = kernel.riscv

RISCV_INCLUDES += -I..
# To switch between g++ and clang, uncomment the line below. g++ is
# the default. To view the disassembly, type `make kernel.dis`

# Clang does not work as well as GCC in this instance. It seems to refuse to inline...
# RISCV_CXX = $(RISCV_CLANGXX)
kernel.riscv: kernel.rvo

RISCV_CCPPFLAGS += -O3 -std=c++14
RISCV_DEFINES += -Dbsg_tiles_X=$(TILE_GROUP_DIM_X)
RISCV_DEFINES += -Dbsg_tiles_Y=$(TILE_GROUP_DIM_Y)
RISCV_DEFINES += -DBLOCK_DIM_X=$(BLOCK_DIM_X)
RISCV_DEFINES += -DBLOCK_DIM_Y=$(BLOCK_DIM_Y)
RISCV_DEFINES += -DBATCH=$(BATCH)
RISCV_DEFINES += -DUNFOLD=$(UNFOLD)
ifeq ($(WARM_CACHE),yes)
RISCV_DEFINES += -DWARM_CACHE
endif
RISCV_LDFLAGS += -flto

include $(EXAMPLES_PATH)/cuda/riscv.mk

###############################################################################
# Execution flow
#
# C_ARGS: Use this to pass arguments that you want to appear in argv
#         For SPMD tests C arguments are: <Path to RISC-V Binary> <Test Name>
#
# SIM_ARGS: Use this to pass arguments to the simulator
###############################################################################
C_ARGS ?= $(BSG_MANYCORE_KERNELS) matmul_blocked

SIM_ARGS ?=

# Include platform-specific execution rules
include $(EXAMPLES_PATH)/execution.mk

###############################################################################
# Regression Flow
###############################################################################

regression: exec.log
	@grep "BSG REGRESSION TEST .*PASSED.*" $< > /dev/null

.DEFAULT_GOAL := help

.PHONY: clean

clean:
