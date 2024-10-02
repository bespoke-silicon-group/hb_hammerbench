ifndef LIBHAMMERBLADE_HELPERS_HOST_MK
LIBHAMMERBLADE_HELPERS_HOST_MK := 1

hammerblade-helpers-dir ?= $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
hammerblade-helpers-host-dir ?= $(hammerblade-helpers-dir)/host

###########################################################
# Interface variables for Makefiles that include this one #
###########################################################
libhammerblade-helpers-host-interface-ldflags   +=
libhammerblade-helpers-host-interface-cxxflags  += -I$(hammerblade-helpers-host-dir)
libhammerblade-helpers-host-interface-libraries +=
libhammerblade-helpers-host-interface-headers   += $(wildcard $(hammerblade-helpers-host-dir)/*.hpp)

endif
