POD_RANGE = $(shell seq -s " " 0 63)

# TEST graph
TESTS += $(call test-name,u16k16,0)
