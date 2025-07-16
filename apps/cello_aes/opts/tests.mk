
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[input],[opt-memcpy],[opt-restrict-ws],[opt-lock],[run])

TESTS += $(call test-name,65536,no,no,no,0)
TESTS += $(call test-name,65536,yes,no,no,0)
TESTS += $(call test-name,65536,yes,yes,no,0)
TESTS += $(call test-name,65536,yes,yes,yes,0)
TESTS += $(call test-name,1024,no,no,no,0)
TESTS += $(call test-name,1024,yes,no,no,0)
TESTS += $(call test-name,1024,yes,yes,no,0)
TESTS += $(call test-name,1024,yes,yes,yes,0)
