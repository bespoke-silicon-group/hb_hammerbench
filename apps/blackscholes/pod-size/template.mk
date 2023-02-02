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
override BSG_MACHINE_PATH = $(REPLICANT_PATH)/machines/pod_X1Y1_ruche_X$(tiles-x)Y$(tiles-y)_hbm_one_pseudo_channel
include $(HB_HAMMERBENCH_PATH)/mk/environment.mk

# Tile Group Dimensions
TILE_GROUP_DIM_X ?= $(BSG_MACHINE_POD_TILES_X)
TILE_GROUP_DIM_Y ?= $(BSG_MACHINE_POD_TILES_Y)

vpath %.c   $(APP_PATH)
vpath %.cpp $(APP_PATH)
vpath %.c   $(APP_PATH)/../opt-pod/
vpath %.cpp $(APP_PATH)/../opt-pod/

# TEST_SOURCES is a list of source files that need to be compiled
TEST_SOURCES = main.cpp bs.cpp

DEFINES += -D_XOPEN_SOURCE=500 -D_BSD_SOURCE -D_DEFAULT_SOURCE -DECB=0 -DCTR=0
DEFINES += -DTILE_GROUP_DIM_X=$(TILE_GROUP_DIM_X)
DEFINES += -DTILE_GROUP_DIM_Y=$(TILE_GROUP_DIM_Y)
DEFINES += -Dbsg_tiles_X=$(TILE_GROUP_DIM_X) -Dbsg_tiles_Y=$(TILE_GROUP_DIM_Y)
DEFINES += -DTWO_DIRS_UP
CDEFINES += 
CXXDEFINES +=

FLAGS     = -g -Wall -Wno-unused-function -Wno-unused-variable
CFLAGS   += -std=gnu99 $(FLAGS)
CXXFLAGS += -std=c++11 $(FLAGS) -Os

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

RISCV_CCPPFLAGS += -O3 -std=c++14
RISCV_CCPPFLAGS += -Dbsg_tiles_X=$(TILE_GROUP_DIM_X)
RISCV_CCPPFLAGS += -Dbsg_tiles_Y=$(TILE_GROUP_DIM_Y)
RISCV_CCPPFLAGS += -DTILE_GROUP_DIM_X=$(TILE_GROUP_DIM_X)
RISCV_CCPPFLAGS += -DTILE_GROUP_DIM_Y=$(TILE_GROUP_DIM_Y)
RISCV_LDFLAGS += -lm

# For some reason clang works much better for Black Scholes
bs.rvo: RISCV_CXX = $(RISCV_CLANGXX)
bs.rvo: RISCV_OPT_LEVEL = -O3
bs.rvo: RISCV_CXXFLAGS += -lm

RISCV_TARGET_OBJECTS += kernel.rvo bs_kernel.rvo

# We put all rodata in scratchpad because otherwise the CLZ lookup
# tables end up in DRAM. Booo!
RISCV_LINK_SCRIPT=$(APP_PATH)/bs_link.ld
BSG_MANYCORE_KERNELS = kernel.riscv

include $(EXAMPLES_PATH)/cuda/riscv.mk
###############################################################################
# Execution flow
#
# C_ARGS: Use this to pass arguments that you want to appear in argv
#         For SPMD tests C arguments are: <Path to RISC-V Binary> <Test Name>
#
# SIM_ARGS: Use this to pass arguments to the simulator
###############################################################################
C_ARGS ?= $(BSG_MANYCORE_KERNELS) bs

SIM_ARGS ?=

# Include platform-specific execution rules
include $(EXAMPLES_PATH)/execution.mk

###############################################################################
# Regression Flow
###############################################################################

regression: exec.log
	@grep "BSG REGRESSION TEST .*PASSED.*" $< > /dev/null

.DEFAULT_GOAL := help


