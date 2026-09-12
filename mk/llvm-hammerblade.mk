# Opt-in HammerBlade LLVM device compilation.
#
# Use after the generated application Makefile so riscv.mk has defined the
# RISCV_CLANG/RISCV_CLANGXX multi-stage compiler functions:
#
#   gmake -f Makefile -f /path/to/mk/llvm-hammerblade.mk \
#     SHELL=/bin/bash '.SHELLFLAGS=-e -o pipefail -c' \
#     RISCV_LLVM_PATH=/path/to/llvm22-build RISCV_LLVM_OBJECT_OUTPUT=1 \
#     RISCV_LLVM_OPT_FLAGS=-enable-dfa-jump-thread main.so main.riscv
#
# Host code and final device linking continue to use the normal Replicant
# toolchain. Only C/C++ sources compiled into RISC-V .rvo objects use LLVM.

ifndef RISCV_LLVM_PATH
$(error RISCV_LLVM_PATH must name an LLVM prefix containing bin/clang, bin/llc, and bin/opt)
endif

RISCV_CC = $(RISCV_CLANG)
RISCV_CXX = $(RISCV_CLANGXX)

# Retain a linker map without changing the benchmark's numerical flags or
# optimization level. This LLVM 10 fork predates Clang's -fstack-usage support.
RISCV_LDFLAGS += -Wl,-Map,kernel.map

# LLVM 10 remains the default assembly-producing path in Replicant's riscv.mk.
# Recent LLVM can instead optimize once, emit an ELF object with its integrated
# assembler, and hand that object to the established GNU runtime/linker. The
# installed binutils 2.32 does not understand modern versioned RISC-V extension
# spellings or LLVM 22 debug relocations, so use LLVM objcopy to remove debug
# information and redundant ISA-attribute metadata before linking.
ifeq ($(RISCV_LLVM_OBJECT_OUTPUT),1)
RISCV_LLVM_OPT_LEVEL ?= -O3
RISCV_LLVM_OPT_FLAGS ?=
RISCV_LLVM_CLANG_IR_FLAGS ?= -Xclang -disable-llvm-passes
RISCV_LLVM_OBJCOPY ?= $(RISCV_LLVM_PATH)/bin/llvm-objcopy
override RISCV_LLVM_LLC_FLAGS += -target-abi=ilp32f -O3

define RISCV_CLANGXX
$(_RISCV_CLANGXX) $(RISCV_CXXFLAGS) $(RISCV_DEFINES) $(RISCV_INCLUDES) \
  $(RISCV_LLVM_CLANG_IR_FLAGS) -c $< -o $@.ll -S -emit-llvm 2>&1 | tee $*.rvo.ll.log && \
$(RISCV_LLVM_OPT) $(RISCV_LLVM_OPT_LEVEL) $(RISCV_LLVM_OPT_FLAGS) \
  -S $@.ll -o $@.opt.ll 2>&1 | tee $*.rvo.opt.log && \
$(RISCV_LLVM_LLC) $(RISCV_LLVM_LLC_FLAGS) -filetype=obj \
  $@.opt.ll -o $@ 2>&1 | tee $*.rvo.obj.log && \
$(RISCV_LLVM_OBJCOPY) --strip-debug --remove-section=.riscv.attributes \
  $@ 2>&1 | tee $*.rvo.log
endef

define RISCV_CLANG
$(_RISCV_CLANG) $(RISCV_CFLAGS) $(RISCV_DEFINES) $(RISCV_INCLUDES) \
  $(RISCV_LLVM_CLANG_IR_FLAGS) -c $< -o $@.ll -S -emit-llvm 2>&1 | tee $*.rvo.ll.log && \
$(RISCV_LLVM_OPT) $(RISCV_LLVM_OPT_LEVEL) $(RISCV_LLVM_OPT_FLAGS) \
  -S $@.ll -o $@.opt.ll 2>&1 | tee $*.rvo.opt.log && \
$(RISCV_LLVM_LLC) $(RISCV_LLVM_LLC_FLAGS) -filetype=obj \
  $@.opt.ll -o $@ 2>&1 | tee $*.rvo.obj.log && \
$(RISCV_LLVM_OBJCOPY) --strip-debug --remove-section=.riscv.attributes \
  $@ 2>&1 | tee $*.rvo.log
endef
endif
