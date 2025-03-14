HB_HAMMERBENCH_PATH ?= $(shell git rev-parse --show-toplevel)
REPLICANT_PATH ?= $(shell cd $(HB_HAMMERBENCH_PATH)/.. && git rev-parse --show-toplevel)

include $(HB_HAMMERBENCH_PATH)/mk/environment.mk

CELLO_HOST_LIB_SOURCES := $(wildcard $(HB_HAMMERBENCH_PATH)/lib/cello/host/*.cpp)
CELLO_HOST_LIB_SOURCES := $(foreach src,$(CELLO_HOST_LIB_SOURCES),$(notdir $(src)))

vpath %.cpp $(HB_HAMMERBENCH_PATH)/lib/cello/host
vpath %.c   $(HB_HAMMERBENCH_PATH)/lib/cello/host

GLOBAL_POINTER_HOST_LIB_SOURCES := $(wildcard $(HB_HAMMERBENCH_PATH)/lib/global_pointer/host/*.cpp)
GLOBAL_POINTER_HOST_LIB_SOURCES := $(foreach src,$(GLOBAL_POINTER_HOST_LIB_SOURCES),$(notdir $(src)))

vpath %.cpp $(HB_HAMMERBENCH_PATH)/lib/global_pointer/host
vpath %.c   $(HB_HAMMERBENCH_PATH)/lib/global_pointer/host

# host sources
TEST_SOURCES += $(CELLO_HOST_LIB_SOURCES)
TEST_SOURCES += $(GLOBAL_POINTER_HOST_LIB_SOURCES)

# host defines
DEFINES += -DHOST
DEFINES += -Dbsg_tiles_X=$(TILE_GROUP_DIM_X) -Dbsg_tiles_Y=$(TILE_GROUP_DIM_Y)
DEFINES += -Dbsg_pods_X=$(BSG_MACHINE_PODS_X) -Dbsg_pods_Y=$(BSG_MACHINE_PODS_Y)

# host flags
CXXFLAGS += -I$(HB_HAMMERBENCH_PATH)/lib

# kernel flags
RISCV_CCPPFLAGS += -I$(HB_HAMMERBENCH_PATH)/lib
RISCV_CCPPFLAGS += -DBSG_COORD_X_WIDTH=$(BSG_MACHINE_NOC_COORD_X_WIDTH)
RISCV_CCPPFLAGS += -DBSG_COORD_Y_WIDTH=$(BSG_MACHINE_NOC_COORD_Y_WIDTH)
RISCV_CCPPFLAGS += -DBSG_POD_TILES_X=$(BSG_MACHINE_POD_TILES_X)
RISCV_CCPPFLAGS += -DBSG_POD_TILES_Y=$(BSG_MACHINE_POD_TILES_Y)
RISCV_CCPPFLAGS += -DBSG_PODS_X=$(BSG_MACHINE_PODS_X)
RISCV_CCPPFLAGS += -DBSG_PODS_Y=$(BSG_MACHINE_PODS_Y)
RISCV_CCPPFLAGS += -fno-rtti
RISCV_CCPPFLAGS += -fno-exceptions
RISCV_CCPPFLAGS += -lstdc++

# kernel sources
CELLO_LIB_SOURCES := $(wildcard $(HB_HAMMERBENCH_PATH)/lib/cello/*.cpp)
CELLO_LIB_OBJECTS := $(CELLO_LIB_SOURCES:.cpp=.rvo)
CELLO_LIB_OBJECTS := $(foreach obj,$(CELLO_LIB_OBJECTS),$(notdir $(obj)))

vpath %.cpp $(HB_HAMMERBENCH_PATH)/lib/cello
vpath %.c   $(HB_HAMMERBENCH_PATH)/lib/cello/bsg_manycore

UTIL_LIB_SOURCES := $(wildcard $(HB_HAMMERBENCH_PATH)/lib/util/*.cpp)
UTIL_LIB_OBJECTS := $(UTIL_LIB_SOURCES:.cpp=.rvo)
UTIL_LIB_OBJECTS := $(foreach obj,$(UTIL_LIB_OBJECTS),$(notdir $(obj)))

vpath %.cpp $(HB_HAMMERBENCH_PATH)/lib/util
vpath %.c   $(HB_HAMMERBENCH_PATH)/lib/util/bsg_manycore

# kernel objects
RISCV_TARGET_OBJECTS += $(CELLO_LIB_OBJECTS)
RISCV_TARGET_OBJECTS += $(UTIL_LIB_OBJECTS)

# put ro data into dmem
LINK_GEN_OPTS += --move_rodata_to_dmem

