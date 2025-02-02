CC := clang
CFLAGS := -Wall -Wextra -g -std=c17
DEFINES := -DS_DEBUG
LDFLAGS := -g
BUILD_DIR := build/linux

XLIB := x11 xi
XCB := xcb xcb-icccm xcb-xinput xcb-xkb xkbcommon-x11
WAYLAND := wayland-client

SUBDIRS := engine testapp tests
CLEAN_TARGETS := $(SUBDIRS:%=clean-%)
BUILD_DIR_TARGETS := $(SUBDIRS:%=build-dir-%)

export BUILD_DIR := $(CURDIR)/$(BUILD_DIR)
export CC
export CFLAGS
export LDFLAGS
export DEFINES

export XLIB
export XCB
export WAYLAND

all: $(SUBDIRS)

clean:
	@echo "Cleaning everything..."
	@rm -rf $(BUILD_DIR)

build-dir: $(BUILD_DIR) $(BUILD_DIR_TARGETS)

$(CLEAN_TARGETS):
	@$(MAKE) -C $(@:clean-%=%) -f Makefile.linux.mak clean

$(SUBDIRS):
	@$(MAKE) -C $@ -f Makefile.linux.mak
	@echo --------------------------------------------

$(BUILD_DIR):
	@echo "Creating directory $(BUILD_DIR)..."
	@mkdir -p $(BUILD_DIR)

$(BUILD_DIR_TARGETS):
	@$(MAKE) -C $(@:build-dir-%=%) -f Makefile.linux.mak build-dir

.PHONY: all clean $(SUBDIRS) $(CLEAN_TARGETS) $(BUILD_DIR_TARGETS)
