# Opt-in HammerBlade LLVM device compilation.
#
# Use after the generated application Makefile so riscv.mk has defined the
# RISCV_CLANG/RISCV_CLANGXX multi-stage compiler functions:
#
#   gmake -f Makefile -f /path/to/mk/llvm-hammerblade.mk \
#     RISCV_LLVM_PATH=/path/to/llvm-install main.so main.riscv
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
