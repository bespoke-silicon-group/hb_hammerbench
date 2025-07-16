
####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[input],[opt-memcpy],[opt-restrict-ws],[opt-lock],[run])

TESTS += $(call test-name,1048576,no,no,no,0)
TESTS += $(call test-name,1048576,yes,no,no,0)
TESTS += $(call test-name,1048576,yes,yes,no,0)
TESTS += $(call test-name,1048576,yes,yes,yes,0)
TESTS += $(call test-name,8192,no,no,no,0)
TESTS += $(call test-name,8192,yes,no,no,0)
TESTS += $(call test-name,8192,yes,yes,no,0)
TESTS += $(call test-name,8192,yes,yes,yes,0)
