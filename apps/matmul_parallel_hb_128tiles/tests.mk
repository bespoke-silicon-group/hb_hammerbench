NS ?= 16 32 64
NITERS ?= 1
PATTERNS ?= 0 1 2
WARM_CACHES ?= no
TILE_X ?= 16
TILE_Y ?= 8

define gen_test
TESTS += N_$(1)__NITER_$(2)__tile-x_$(TILE_X)__tile-y_$(TILE_Y)__PATTERN_$(3)__warm-cache_$(4)

N_$(1)__NITER_$(2)__tile-x_$(TILE_X)__tile-y_$(TILE_Y)__PATTERN_$(3)__warm-cache_$(4)/parameters.mk:
N = $(1)
NITER = $(2)
tile-x = $(TILE_X)
tile-y = $(TILE_Y)
PATTERN = $(3)
warm-cache = $(4)
endef

$(foreach n,$(NS), \
  $(foreach it,$(NITERS), \
    $(foreach p,$(PATTERNS), \
      $(foreach wc,$(WARM_CACHES), \
        $(eval $(call gen_test,$(n),$(it),$(p),$(wc))) \
      ) \
    ) \
  ) \
)
