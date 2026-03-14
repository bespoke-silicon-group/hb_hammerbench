HB_HAMMERBENCH_PATH:=$(shell git rev-parse --show-toplevel)
GRAPH_PATH = $(HB_HAMMERBENCH_PATH)/apps/graph_data

GRAPH_NAME ?= no-name-given
ROOT ?= 0
NUMPODS ?= 1
TILE_GROUP_DIM_X ?= 4
TILE_GROUP_DIM_Y ?= 4

# Parameters for graph generation
NUM_VERTICES ?= 16
NUM_EDGES ?= 32
WEIGHT_LOW ?= 1
WEIGHT_HIGH ?= 10
ALLOW_SELF ?= False

preprocess:
	ln -sf $(GRAPH_PATH)/$(GRAPH_NAME).mtx
	python convert_mtx.py $(GRAPH_NAME).mtx
	python make_config.py \
		--graph $(GRAPH_NAME).mtx \
		--out ../config.$(GRAPH_NAME)-rev.mk \
		--root $(ROOT) \
		--numpods $(NUMPODS) \
		--tgx $(TILE_GROUP_DIM_X) \
		--tgy $(TILE_GROUP_DIM_Y)

generate:
	python generate_graph2.py \
		$(GRAPH_NAME)-rev.mtx \
		$(NUM_VERTICES) \
		$(NUM_EDGES) \
		$(WEIGHT_LOW) \
		$(WEIGHT_HIGH) \
		$(ALLOW_SELF)
	python make_config.py \
		--graph $(GRAPH_NAME)-rev.mtx \
		--out ../config.$(GRAPH_NAME)-rev.mk \
		--root $(ROOT) \
		--numpods $(NUMPODS) \
		--tgx $(TILE_GROUP_DIM_X) \
		--tgy $(TILE_GROUP_DIM_Y)

clean:
	rm -f *.mtx
	rm -f ../config.*
