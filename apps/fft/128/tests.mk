# $(call test-name,[num-iter],[warm-cache])
TESTS += $(call test-name,1,yes)
TESTS += $(call test-name,3,yes)
TESTS += $(call test-name,8,no)
