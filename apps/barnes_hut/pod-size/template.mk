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

###############################################################################
# Host code compilation flags and flow
###############################################################################
# import parameters and APP_PATH

# Tile Group Dimensions
TILE_GROUP_DIM_X ?= $(BSG_MACHINE_POD_TILES_X)
TILE_GROUP_DIM_Y ?= $(BSG_MACHINE_POD_TILES_Y)

BARNES_HUT_PATH := $(HB_HAMMERBENCH_PATH)/apps/barnes_hut

vpath %.cpp $(BARNES_HUT_PATH)/Galois/lonestar/liblonestar/src/
vpath %.c   $(APP_PATH)
vpath %.cpp $(APP_PATH)
vpath %.c   $(APP_PATH)/../
vpath %.cpp $(APP_PATH)/../

# TEST_SOURCES is a list of source files that need to be compiled
TEST_SOURCES   = Barneshut.cpp  
TEST_SOURCES  += BoilerPlate.cpp
TEST_HEADERS  := Node.hpp Point.hpp Body.hpp Config.hpp
TEST_HEADERS  := $(addprefix $(BARNES_HUT_PATH)/,$(TEST_HEADERS))

DEFINES += -DTILE_GROUP_DIM_X=$(TILE_GROUP_DIM_X)
DEFINES += -DTILE_GROUP_DIM_Y=$(TILE_GROUP_DIM_Y)
DEFINES += -D_XOPEN_SOURCE=500 -D_BSD_SOURCE -D_DEFAULT_SOURCE
CDEFINES += -Dbsg_tiles_X=$(TILE_GROUP_DIM_X) -Dbsg_tiles_Y=$(TILE_GROUP_DIM_Y)
CXXDEFINES +=

INCLUDES  = -I$(BARNES_HUT_PATH)/Galois/install/include -I$(BARNES_HUT_PATH)/Galois/lonestar/liblonestar/include/
INCLUDES += -I$(shell llvm-config-11-64  --includedir) -I$(BARNES_HUT_PATH)/.
FLAGS     = -O3 -g -Wall -Wno-unused-function -Wno-unused-variable -Wno-strict-aliasing -Wno-deprecated-declarations
CFLAGS   += -std=c99 $(FLAGS)
CXXFLAGS += -std=c++17 $(FLAGS)
LDFLAGS  = -lpthread -Wl,-rpath=$(BARNES_HUT_PATH)/Galois/install/lib64 -L$(BARNES_HUT_PATH)/Galois/install/lib64 -lgalois_shmem -lnuma $(shell llvm-config-11-64 --ldflags --libs)

# compilation.mk defines rules for compilation of C/C++
include $(EXAMPLES_PATH)/compilation.mk

###############################################################################
# Host code link flags and flow
###############################################################################

LDFLAGS +=

# link.mk defines rules for linking of the final execution binary.
include $(EXAMPLES_PATH)/link.mk
$(TEST_OBJECTS): $(TEST_HEADERS)

###############################################################################
# Device code compilation flow
###############################################################################

# BSG_MANYCORE_KERNELS is a list of manycore executables that should
# be built before executing.

update.rvo: RISCV_CXX = $(RISCV_CLANGXX)
forces.rvo: RISCV_CXX = $(RISCV_GCC)
build.rvo: RISCV_CXX = $(RISCV_CLANGXX)

RISCV_CCPPFLAGS += -O3 -std=c++14
RISCV_CCPPFLAGS += -Dbsg_tiles_X=$(TILE_GROUP_DIM_X)
RISCV_CCPPFLAGS += -Dbsg_tiles_Y=$(TILE_GROUP_DIM_Y)
RISCV_DEFINES += -DTILE_GROUP_DIM_X=$(TILE_GROUP_DIM_X)
RISCV_DEFINES += -DTILE_GROUP_DIM_Y=$(TILE_GROUP_DIM_Y)
RISCV_DEFINES += -DRISCV
RISCV_CCPPFLAGS += -D__KERNEL__ -O3
RISCV_INCLUDES  += -I$(BARNES_HUT_PATH)
RISCV_TARGET_OBJECTS = update.rvo forces.rvo build.rvo summarize.rvo
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
PHASE    ?= forces
POD_ID_X ?= 0
POD_ID_Y ?= 0
NPODS_X  ?= 1
NPODS_Y  ?= 1
NBODIES  ?= 1024

C_ARGS   ?= -n $(NBODIES) -steps 1 -t 1 -phase $(PHASE) -px $(POD_ID_X) -py $(POD_ID_Y) -npx $(NPODS_X) -npy $(NPODS_Y)
SIM_ARGS ?=

# Include platform-specific execution rules
include $(EXAMPLES_PATH)/execution.mk

###############################################################################
# Regression Flow
###############################################################################

regression: exec.log
	@grep "BSG REGRESSION TEST .*PASSED.*" $< > /dev/null

.DEFAULT_GOAL := help


