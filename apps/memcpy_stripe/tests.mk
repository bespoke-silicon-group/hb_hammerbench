MSIZE = 524288
TESTS += $(call test-name,16,8,$(MSIZE),no,1)
TESTS += $(call test-name,16,8,$(MSIZE),no,2)
TESTS += $(call test-name,16,8,$(MSIZE),no,4)
TESTS += $(call test-name,16,8,$(MSIZE),no,8)
TESTS += $(call test-name,16,8,$(MSIZE),no,16)
TESTS += $(call test-name,16,8,$(MSIZE),no,32)
