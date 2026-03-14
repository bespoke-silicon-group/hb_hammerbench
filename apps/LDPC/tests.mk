# # data-size (z-factor: 8, 16, 32, or 128), warm-cache
TESTS += $(call test-name,8,no)
TESTS += $(call test-name,8,yes)
TESTS += $(call test-name,16,no)
TESTS += $(call test-name,16,yes)
TESTS += $(call test-name,32,no)
TESTS += $(call test-name,32,yes)
TESTS += $(call test-name,128,no)
TESTS += $(call test-name,128,yes)

