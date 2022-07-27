# Run `make checkout_graphit`
HB_HAMMERBENCH_PATH:=$(shell git rev-parse --show-toplevel)
include $(HB_HAMMERBENCH_PATH)/mk/environment.mk
PAGERANK_PATH:=$(HB_HAMMERBENCH_PATH)/apps/pagerank
GRAPHIT_PATH:=$(HB_HAMMERBENCH_PATH)/apps/pagerank/graphit

.PHONY: checkout_graphit
checkout_graphit:
	cd $(PAGERANK_PATH) && git clone git@github.com:bespoke-silicon-group/graphit
	cd $(GRAPHIT_PATH) && git checkout pagerank

