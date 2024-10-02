ifndef LIBHAMMERBLADE_HELPERS_DEVICE_MK
LIBHAMMERBLADE_HELPERS_DEVICE_MK := 1

hammerblade-helpers-dir ?= $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
hammerblade-helpers-device-dir ?= $(hammerblade-helpers-dir)/device

bsg-manycore-lib-dir ?= $(BSG_MANYCORE_DIR)/software/bsg_manycore_lib

libhammerblade-helpers-device-headers   +=
libhammerblade-helpers-device-libraries +=
libhammerblade-helpers-device-cxxflags  += -I$(bsg-manycore-lib-dir)
libhammerblade-helpers-device-cxxflags  += -I$(hammerblade-helpers-device-dir)
libhammerblade-helpers-device-ldflags   +=
###########################################################
# Interface variables for Makefiles that include this one #
###########################################################

# should be used by executables/libraries to link with this library
libhammerblade-helpers-device-interface-ldflags   +=

# should be used by executables/libraries to compile with this library
libhammerblade-helpers-device-interface-cxxflags  += -I$(bsg-manycore-lib-dir)
libhammerblade-helpers-device-interface-cxxflags  += -I$(hammerblade-helpers-device-dir)

# should be marked as dependencies by executables/libraries that use this library
libhammerblade-helpers-device-interface-libraries +=

# should be marked as dependencies by executables/libraries that use this library
libhammerblade-helpers-device-interface-headers   += $(wildcard $(hammerblade-helpers-device-dir)/*.hpp)
#libhammerblade-helpers-device-interface-sources   += $(wildcard $(hammerblade-helpers-device-dir)/*.cpp)

endif
