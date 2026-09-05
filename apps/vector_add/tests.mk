# tile-x, tile-y, vector-size, warm-cache
TILE_X ?= 16
TILE_Y ?= 8
VECTOR_SIZE ?= 65536
TESTS += $(call test-name,$(TILE_X),$(TILE_Y),$(VECTOR_SIZE),yes)
TESTS += $(call test-name,$(TILE_X),$(TILE_Y),$(VECTOR_SIZE),no)
