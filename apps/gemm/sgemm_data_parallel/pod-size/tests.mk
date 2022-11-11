####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[size-m],[size-n],[size-p],[warm-cache],[prefetch],[tiles-x],[tiles-y])
TESTS +=  $(call test-name,512,512,512,no,no,4,2)
TESTS +=  $(call test-name,512,512,512,no,no,4,4)
TESTS +=  $(call test-name,512,512,512,no,no,8,4)
TESTS +=  $(call test-name,512,512,512,no,no,8,8)
TESTS +=  $(call test-name,512,512,512,no,no,16,8)
TESTS +=  $(call test-name,512,512,512,no,no,16,16)


