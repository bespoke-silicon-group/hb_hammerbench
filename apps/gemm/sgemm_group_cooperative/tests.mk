# (size-m, size-n, size-p, warm-cache)
# the original setting
TESTS +=  $(call test-name,128,256,256,no)
# cache resident setting
TESTS +=  $(call test-name,256,256,256,yes)
# cache non resident setting
TESTS +=  $(call test-name,512,512,512,no)
