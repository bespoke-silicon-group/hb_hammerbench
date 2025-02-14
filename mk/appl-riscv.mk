HB_HAMMERBENCH_PATH ?= $(shell git rev-parse --show-toplevel)
include $(HB_HAMMERBENCH_PATH)/mk/environment.mk
include $(BSG_MACHINE_PATH)/Makefile.machine.include

APPL_DIR       ?= $(HB_HAMMERBENCH_PATH)/lib/appl
APPLRTS_DIR    ?= $(HB_HAMMERBENCH_PATH)/lib/applrts
APPLSTATIC_DIR ?= $(HB_HAMMERBENCH_PATH)/lib/applstatic
LIGRA_DIR      ?= $(HB_HAMMERBENCH_PATH)/lib/ligra

RISCV_CXXFLAGS += -I$(APPL_DIR)
RISCV_CXXFLAGS += -I$(APPLRTS_DIR)
RISCV_CXXFLAGS += -I$(APPLSTATIC_DIR)
RISCV_CXXFLAGS += -I$(LIGRA_DIR)

#BSG_ELF_STACK_PTR ?= 0x0003fffc


vpath %.cpp $(APPL_DIR)
vpath %.c   $(APPL_DIR)
vpath %.S   $(APPL_DIR)
vpath %.cpp $(APPLRTS_DIR)
vpath %.c   $(APPLRTS_DIR)
vpath %.S   $(APPLRTS_DIR)
vpath %.cpp $(APPLSTATIC_DIR)
vpath %.c   $(APPLSTATIC_DIR)
vpath %.S   $(APPLSTATIC_DIR)
vpath %.cpp $(LIGRA_DIR)
vpath %.c   $(LIGRA_DIR)
vpath %.S   $(LIGRA_DIR)

# APPL implemenation
ifeq ($(APPL_IMPL), APPL_IMPL_APPLRTS)
	RISCV_CXXFLAGS  +=-DAPPL_IMPL_APPLRTS
	RISCV_CXXFLAGS  +=-fno-rtti
	RISCV_TARGET_OBJECTS += appl-runtime.rvo
	RISCV_TARGET_OBJECTS += appl-malloc.rvo
	RISCV_TARGET_OBJECTS += applrts-config.rvo
	RISCV_TARGET_OBJECTS += applrts-runtime.rvo
	RISCV_TARGET_OBJECTS += applrts-scheduler.rvo
	RISCV_TARGET_OBJECTS += applrts-stats.rvo
endif

ifeq ($(APPL_IMPL), APPL_IMPL_SERIAL)
	RISCV_CXXFLAGS += -DAPPL_IMPL_SERIAL
	RISCV_TARGET_OBJECTS += appl-malloc.rvo
endif

ifeq ($(APPL_IMPL), APPL_IMPL_STATIC)
	RISCV_CXXFLAGS += -DAPPL_IMPL_STATIC
	RISCV_CXXFLAGS  +=-fno-rtti
	RISCV_TARGET_OBJECTS += appl-runtime.rvo
	RISCV_TARGET_OBJECTS += appl-malloc.rvo
	RISCV_TARGET_OBJECTS += applstatic-config.rvo
	RISCV_TARGET_OBJECTS += applstatic-runtime.rvo
endif

ifeq ($(APPL_IMPL), APPL_IMPL_CELLO)
	CELLO_DIR := $(CL_DIR)/../cello
	RISCV_CCPPFLAGS +=-DAPPL_IMPL_CELLO
	RISCV_CCPPFLAGS += -I$(CELLO_DIR)/include
	RISCV_CCCPPFLAGS += -fno-threadsafe-statics
	vpath %.cpp $(CELLO_DIR)/src
	vpath %.c   $(CELLO_DIR)/src
	RISCV_TARGET_OBJECTS += appl-runtime.rvo
	RISCV_TARGET_OBJECTS += cello_invoke.rvo
	RISCV_TARGET_OBJECTS += cello_malloc.rvo
	RISCV_TARGET_OBJECTS += cello_scheduler.rvo
endif

# include riscv builddefs
CRT_PATH := $(HB_HAMMERBENCH_PATH)/lib/appl
include $(EXAMPLES_PATH)/cuda/riscv.mk

RISCV_LD = $(_RISCV_GXX)


