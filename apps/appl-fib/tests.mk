####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[fib-in],[grain-size],[tgx],[tgy],[appl-impl])
#TESTS += $(call test-name,10,1,1,1,APPL_SERIAL)
#TESTS += $(call test-name,10,1,1,1,APPL_APPLRTS)
TESTS += $(call test-name,5,2,1,1,APPL_APPLRTS)
TESTS += $(call test-name,5,2,2,1,APPL_APPLRTS)
TESTS += $(call test-name,5,2,4,1,APPL_APPLRTS)
TESTS += $(call test-name,5,2,1,2,APPL_APPLRTS)
TESTS += $(call test-name,5,2,2,2,APPL_APPLRTS)
#TESTS += $(call test-name,5,1,4,2,APPL_APPLRTS)


