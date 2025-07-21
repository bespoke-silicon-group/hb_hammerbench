
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[num-iter],[tiles-x],[tiles-y],[pods-x],[pods-y],[opt-memcpy],[opt-restrict-ws],[opt-lock],[opt-rng],[opt-icache],[run])

TESTS += $(call test-name,email-Enron,16,8,4,2,no,yes,no,no,no,0)
TESTS += $(call test-name,email-Enron,16,8,4,2,yes,yes,no,no,no,0)
TESTS += $(call test-name,email-Enron,16,8,4,2,no,yes,no,no,no,0)
TESTS += $(call test-name,email-Enron,16,8,4,2,no,yes,yes,no,no,0)
TESTS += $(call test-name,email-Enron,16,8,4,2,no,yes,no,yes,no,0)
TESTS += $(call test-name,email-Enron,16,8,4,2,no,yes,no,no,yes,0)
TESTS += $(call test-name,roadNet-CA,16,8,4,2,no,yes,no,no,no,0)
TESTS += $(call test-name,roadNet-CA,16,8,4,2,yes,yes,no,no,no,0)
TESTS += $(call test-name,roadNet-CA,16,8,4,2,no,yes,no,no,no,0)
TESTS += $(call test-name,roadNet-CA,16,8,4,2,no,yes,yes,no,no,0)
TESTS += $(call test-name,roadNet-CA,16,8,4,2,no,yes,no,yes,no,0)
TESTS += $(call test-name,roadNet-CA,16,8,4,2,no,yes,no,no,yes,0)
