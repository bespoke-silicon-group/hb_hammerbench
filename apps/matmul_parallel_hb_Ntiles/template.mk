## matmul_serial_hb template
# This Makefile structure is modelled after the memcpy example so that
# `make profile.log` and related targets work even when the CAD environment
# is not sourced.  It defines the host/device build rules and execution
# flow used by HammerBench apps.

# include per-test parameters and generated paths
include parameters.mk
include app_path.mk

PATTERN ?= 0
warm-cache ?= no

# root of the HammerBench repository
HB_HAMMERBENCH_PATH:=$(shell git rev-parse --show-toplevel)

# default machine path; tests can override by setting tile-x/tile-y if needed
override BSG_MACHINE_PATH = $(REPLICANT_PATH)/machines/pod_X1Y1_ruche_X$(tile-x)Y$(tile-y)_hbm_one_pseudo_channel

# environment setup (paths to libraries, tools, etc.)
include $(HB_HAMMERBENCH_PATH)/mk/environment.mk

# --- host code compilation flags ---

# Tile Group Dimensions (parallel run uses configured tile-x/tile-y)
TILE_GROUP_DIM_X ?= $(tile-x)
TILE_GROUP_DIM_Y ?= $(tile-y)

vpath %.c   $(APP_PATH)
vpath %.cpp $(APP_PATH)

# sources, overrides in parameters.mk if necessary
TEST_SOURCES = main.cpp

DEFINES += -D_XOPEN_SOURCE=500 -D_BSD_SOURCE -D_DEFAULT_SOURCE
DEFINES += -DN=$(N) -DNITER=$(NITER) -DPATTERN=$(PATTERN)
CDEFINES += -Dbsg_tiles_X=$(TILE_GROUP_DIM_X) -Dbsg_tiles_Y=$(TILE_GROUP_DIM_Y)
CDEFINES += -DCACHE_LINE_WORDS=$(BSG_MACHINE_VCACHE_LINE_WORDS)

# Expose tile dimensions to host compiler so host code can reference
# `bsg_tiles_X` / `bsg_tiles_Y` without relying on CAD-derived flags.
DEFINES += -Dbsg_tiles_X=$(tile-x) -Dbsg_tiles_Y=$(tile-y)

FLAGS     = -g -Wall -Wno-unused-function -Wno-unused-variable
CFLAGS   += -std=c99 $(FLAGS)
CXXFLAGS += -std=c++11 $(FLAGS)
ifeq ($(warm-cache),yes)
FLAGS += -DWARM_CACHE
endif

# compilation rules from HammerBench examples
include $(EXAMPLES_PATH)/compilation.mk

# --- host linking ---
LDFLAGS +=
include $(EXAMPLES_PATH)/link.mk

# --- device code (RISC-V) ---

RISCV_CCPPFLAGS += -O3 -std=c++14
RISCV_CCPPFLAGS += -Dbsg_tiles_X=$(TILE_GROUP_DIM_X)
RISCV_CCPPFLAGS += -Dbsg_tiles_Y=$(TILE_GROUP_DIM_Y)
RISCV_CCPPFLAGS += -DCACHE_LINE_WORDS=$(BSG_MACHINE_VCACHE_LINE_WORDS)
# pass problem-specific parameters to the kernel
RISCV_CCPPFLAGS += -DN=$(N) -DNITER=$(NITER) -DPATTERN=$(PATTERN)
ifeq ($(warm-cache),yes)
RISCV_CCPPFLAGS += -DWARM_CACHE
endif
RISCV_LDFLAGS += -flto
RISCV_TARGET_OBJECTS = kernel.rvo
BSG_MANYCORE_KERNELS = main.riscv
include $(EXAMPLES_PATH)/cuda/riscv.mk

# --- execution flow ---
C_ARGS ?= $(BSG_MANYCORE_KERNELS) matmul
SIM_ARGS ?=
include $(EXAMPLES_PATH)/execution.mk

# regression helper
regression: exec.log
	@grep "BSG REGRESSION TEST .*PASSED.*" $< > /dev/null

.DEFAULT_GOAL := help
