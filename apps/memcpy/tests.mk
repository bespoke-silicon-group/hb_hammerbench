#########################################################
# CHANGE ME: TESTS				        #
# TESTS += $(call test-name,[buffer-size],[warm-cache]) #
#########################################################
MSIZE ?= 524288
#MSIZE = 4096
TILE_X ?= 16
TILE_Y ?= 8
TESTS += $(call test-name,$(TILE_X),$(TILE_Y),$(MSIZE),no)
