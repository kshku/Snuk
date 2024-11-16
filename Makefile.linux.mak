CC := clang
CFLAGS := -Wall -Wextra -g
DEFINES := -DS_DEBUG
LDFLAGS := -g
BUILD_DIR := build

SUBDIRS := engine testapp tests
CLEAN_TARGETS := $(SUBDIRS:%=clean-%)

export BUILD_DIR := $(CURDIR)/$(BUILD_DIR)
export CC
export CFLAGS
export LDFLAGS
export DEFINES

all: $(BUILD_DIR) $(SUBDIRS)

clean:
	@echo "Cleaning everything..."
	@rm -rf $(BUILD_DIR)

$(CLEAN_TARGETS):
	@$(MAKE) -C $(@:clean-%=%) -f Makefile.linux.mak clean

$(SUBDIRS): | $(BUILD_DIR)
	@$(MAKE) -C $@ -f Makefile.linux.mak
	@echo --------------------------------------------

$(BUILD_DIR):
	@echo "Creating directory $(BUILD_DIR)..."
	@mkdir -p $(BUILD_DIR)

.PHONY: all clean $(SUBDIRS) $(CLEAN_TARGETS)
