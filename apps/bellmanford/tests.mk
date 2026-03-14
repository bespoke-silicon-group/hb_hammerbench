# NUMPODS
POD_RANGE = $(shell seq -s " " 0 63)

TESTS += $(call test-name,16-rev,1)
TESTS += $(call test-name,50-rev,1)
TESTS += $(call test-name,100-rev,1)
