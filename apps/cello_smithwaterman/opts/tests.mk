
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[num-iter],[tiles-x],[tiles-y],[pods-x],[pods-y],[opt-memcpy],[opt-restrict-ws],[opt-lock],[opt-rng],[opt-icache],[run])

TESTS += $(call test-name,65536,16,8,4,2,no,no,no,no,no,0)
TESTS += $(call test-name,65536,16,8,4,2,yes,no,no,no,no,0)
TESTS += $(call test-name,65536,16,8,4,2,no,yes,no,no,no,0)
TESTS += $(call test-name,65536,16,8,4,2,no,no,yes,no,no,0)
TESTS += $(call test-name,65536,16,8,4,2,no,no,no,yes,no,0)
TESTS += $(call test-name,65536,16,8,4,2,no,no,no,no,yes,0)
