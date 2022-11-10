####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[nx],[ny],[nz],[num-iter],[warm-cache],[tiles-x],[tiles-y])
TESTS += $(call test-name,32,16,512,1,no,4,2)
TESTS += $(call test-name,32,16,512,1,no,4,4)
TESTS += $(call test-name,32,16,512,1,no,8,4)
TESTS += $(call test-name,32,16,512,1,no,8,8)
#TESTS += $(call test-name,32,16,512,1,no,16,8)
TESTS += $(call test-name,32,16,512,1,no,16,16)
