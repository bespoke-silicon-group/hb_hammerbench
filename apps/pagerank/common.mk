# Run `make checkout_graphit`
HB_HAMMERBENCH_PATH:=$(shell git rev-parse --show-toplevel)
include $(HB_HAMMERBENCH_PATH)/mk/environment.mk
PAGERANK_PATH:=$(HB_HAMMERBENCH_PATH)/apps/pagerank
GRAPHIT_PATH:=$(HB_HAMMERBENCH_PATH)/apps/pagerank/graphit
GAPBS_PATH:=$(HB_HAMMERBENCH_PATH)/apps/pagerank/gapbs

.PHONY: checkout_graphit
checkout_graphit:
	cd $(PAGERANK_PATH) && git clone git@github.com:bespoke-silicon-group/graphit
	cd $(GRAPHIT_PATH) && git checkout pagerank

checkout_gapbs:
	cd $(PAGERANK_PATH) && git clone git@github.com:sbeamer/gapbs
	cd $(GAPBS_PATH) && make -j

%.el: %.mtx
	$(GAPBS_PATH)/converter -f $^ -e $@


# wiki-Vote
wiki-Vote.tar.gz:
	wget "https://suitesparse-collection-website.herokuapp.com/MM/SNAP/wiki-Vote.tar.gz"

wiki-Vote/wiki-Vote.mtx: wiki-Vote.tar.gz
	tar -xf wiki-Vote.tar.gz


all-graphs: wiki-Vote/wiki-Vote.el
