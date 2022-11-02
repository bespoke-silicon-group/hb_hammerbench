#ASIZE = 67108864
WARMCACHE = yes
TESTS += $(call test-name,4,2,65536,$(WARMCACHE))
TESTS += $(call test-name,4,4,65536,$(WARMCACHE))
TESTS += $(call test-name,8,4,131072,$(WARMCACHE))
TESTS += $(call test-name,8,8,131072,$(WARMCACHE))
TESTS += $(call test-name,16,8,262144,$(WARMCACHE))
TESTS += $(call test-name,16,16,262144,$(WARMCACHE))
