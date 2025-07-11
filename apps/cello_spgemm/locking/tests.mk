
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[mtx-a],[mtx-b],[tiles-x],[tiles-y],[pods-x],[pods-y],[exponential_backoff],[try_lock],[run])

TESTS += $(call test-name,email-Enron,email-Enron,16,8,4,2,no,no,0)
TESTS += $(call test-name,roadNet-CA,roadNet-CA,16,8,4,2,no,no,0)
TESTS += $(call test-name,email-Enron,email-Enron,16,8,4,2,no,yes,0)
TESTS += $(call test-name,roadNet-CA,roadNet-CA,16,8,4,2,no,yes,0)
TESTS += $(call test-name,email-Enron,email-Enron,16,8,4,2,yes,no,0)
TESTS += $(call test-name,roadNet-CA,roadNet-CA,16,8,4,2,yes,no,0)
TESTS += $(call test-name,email-Enron,email-Enron,16,8,4,2,yes,yes,0)
TESTS += $(call test-name,roadNet-CA,roadNet-CA,16,8,4,2,yes,yes,0)
