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

$(GAPBS_PATH)/converter:
	make -j -f $(GAPBS_PATH)

%.el: %.mtx $(GAPBS_PATH)/converter
	$(GAPBS_PATH)/converter -f $< -e $@

cit-HepTh.tar.gz:
	wget "https://suitesparse-collection-website.herokuapp.com/MM/SNAP/cit-HepTh.tar.gz"

wiki-Vote.tar.gz:
	wget "https://suitesparse-collection-website.herokuapp.com/MM/SNAP/wiki-Vote.tar.gz"

rgg_n_2_15_s0.tar.gz:
	wget "https://suitesparse-collection-website.herokuapp.com/MM/DIMACS10/rgg_n_2_15_s0.tar.gz"

p2p-Gnutella09.tar.gz:
	wget "https://suitesparse-collection-website.herokuapp.com/MM/SNAP/p2p-Gnutella09.tar.gz"

email-Enron.tar.gz:
	wget "https://suitesparse-collection-website.herokuapp.com/MM/SNAP/email-Enron.tar.gz"

soc-Pokec.tar.gz:
	wget "https://suitesparse-collection-website.herokuapp.com/MM/SNAP/soc-Pokec.tar.gz"

hollywood-2009.tar.gz:
	wget "https://suitesparse-collection-website.herokuapp.com/MM/LAW/hollywood-2009.tar.gz"

ljournal-2008.tar.gz:
	wget "https://suitesparse-collection-website.herokuapp.com/MM/LAW/ljournal-2008.tar.gz"

roadNet-CA.tar.gz:
	wget "https://suitesparse-collection-website.herokuapp.com/MM/SNAP/roadNet-CA.tar.gz"

road_central.tar.gz:
	wget "https://suitesparse-collection-website.herokuapp.com/MM/DIMACS10/road_central.tar.gz"

road_usa.tar.gz:
	wget "https://suitesparse-collection-website.herokuapp.com/MM/DIMACS10/road_usa.tar.gz"



%.mtx: %.tar.gz
	tar -xf $< --strip-components 1 $(basename $@)/$@


all-graphs: wiki-Vote.el soc-Pokec.el p2p-Gnutella09.el email-Enron.el hollywood-2009.el \
	road_usa.el road_central.el roadNet-CA.el ljournal-2008.el cit-HepTh.el
