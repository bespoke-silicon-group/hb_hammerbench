
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[input],[opt-memcpy],[opt-restrict-ws],[opt-lock],[run])

TESTS += $(call test-name,16,no,no,no,0)
TESTS += $(call test-name,16,yes,no,no,0)
TESTS += $(call test-name,16,yes,yes,no,0)
TESTS += $(call test-name,16,yes,yes,yes,0)
TESTS += $(call test-name,1,no,no,no,0)
TESTS += $(call test-name,1,yes,no,no,0)
TESTS += $(call test-name,1,yes,yes,no,0)
TESTS += $(call test-name,1,yes,yes,yes,0)
