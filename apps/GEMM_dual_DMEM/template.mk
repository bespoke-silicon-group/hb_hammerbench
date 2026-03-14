include parameters.mk
include app_path.mk

HB_HAMMERBENCH_PATH := $(shell git rev-parse --show-toplevel)
include $(HB_HAMMERBENCH_PATH)/mk/environment.mk

# tile group
TILE_GROUP_DIM_X ?= $(tgx)
TILE_GROUP_DIM_Y ?= $(tgy)

vpath %.c   $(APP_PATH)
vpath %.cpp $(APP_PATH)

# Host
test-name ?= gemm
KERNEL_NAME = matmul_ab_dmem
TEST_SOURCES := main.c

DEFINES  += -D_XOPEN_SOURCE=500 -D_BSD_SOURCE -D_DEFAULT_SOURCE
FLAGS     = -g -Wall -Wno-unused-function -Wno-unused-variable
CFLAGS   += -std=c99   $(FLAGS)
CXXFLAGS += -std=c++11 $(FLAGS)

include $(EXAMPLES_PATH)/compilation.mk
include $(EXAMPLES_PATH)/link.mk

# Device
BSG_MANYCORE_KERNELS = kernel.riscv
kernel.riscv: kernel.rvo

RISCV_DEFINES += -Dbsg_tiles_X=$(TILE_GROUP_DIM_X)
RISCV_DEFINES += -Dbsg_tiles_Y=$(TILE_GROUP_DIM_Y)
RISCV_CCPPFLAGS += -O3 -std=c++14

include $(EXAMPLES_PATH)/cuda/riscv.mk

# Execution
C_ARGS ?= $(BSG_MANYCORE_KERNELS) $(KERNEL_NAME)
SIM_ARGS ?=

include $(EXAMPLES_PATH)/execution.mk
RUN_RULES += profile.log
RUN_RULES += exec.log
.DEFAULT_GOAL := help

regression: exec.log
	@grep "BSG REGRESSION TEST .*PASSED.*" $< > /dev/null
