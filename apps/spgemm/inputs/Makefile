HB_HAMMERBENCH_PATH:=$(shell git rev-parse --show-toplevel)
GRAPH_PATH = $(HB_HAMMERBENCH_PATH)/apps/graph_data
PYTHON ?= python3

GRAPH_NAME?=wiki-Vote
include ../config.$(GRAPH_NAME).mk

preprocess:
	ln -sf $(GRAPH_PATH)/$(GRAPH_NAME).mtx
	$(PYTHON) convert_mtx.py $(GRAPH_NAME).mtx
