TESTS += $(call test-name,64,8,512,no)
TESTS += $(call test-name,32,8,512,yes)

# Two Z chunks and multiple XY blocks exercise all DRAM-neighbor directions.
TESTS += $(call test-name,32,16,1024,no)
