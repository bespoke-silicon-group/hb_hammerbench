# Minimal app_template.mk stub for environments without full CAD tools
# This allows `make profile.log` and similar simple targets to run
# It is a fallback and does not perform real CAD or simulation work.

# If the test didn't define a machine, set a harmless default name so
# hardware.mk doesn't bail during parsing.  The stub profile.log rule
# doesn't actually use it.
ifndef BSG_MACHINE_NAME
BSG_MACHINE_NAME := dummy
endif

ifndef HB_HAMMERBENCH_PATH
HB_HAMMERBENCH_PATH:=$(shell git rev-parse --show-toplevel)
endif

include $(HB_HAMMERBENCH_PATH)/mk/environment.mk

ifndef APP_PATH
APP_PATH:=$(HB_HAMMERBENCH_PATH)/apps
endif

.PHONY: profile.log generate purge

# Fallback profile.log: create a placeholder so `make profile.log` doesn't fail.
profile.log:
	@echo "BSG MAKE WARN: CAD environment not configured; creating placeholder profile.log" 1>&2 || true
	@echo "# placeholder profile.log" > profile.log
	@echo "# To generate real profiles, configure bsg_cadenv and run make generate/profile.log" >> profile.log
	@echo "Created placeholder profile.log"

# Minimal generate/purge targets to mirror upstream behaviour
generate:
	@mkdir -p $(CURDIR)/
	@echo "Creating $(notdir $(CURDIR))/app_path.mk"
	@echo "APP_PATH = $(APP_PATH)/$(notdir $(CURDIR))" > $(CURDIR)/app_path.mk
	@echo "Created placeholder generated files"

purge:
	rm -f profile.log
	@echo "Removed placeholder profile.log"
