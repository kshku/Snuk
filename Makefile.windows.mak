SHELL := powershell.exe
.SHELLFLAGS := -NoProfile -NoInteractive -Command

CC := clang
CFLAGS := -Wall -Wextra -g -std=c17
DEFINES := -DS_DEBUG
LDFLAGS := -g
BUILD_DIR := build/windows

SUBDIRS := engine testapp tests
CLEAN_TARGETS := $(SUBDIRS:%=clean-%)
BUILD_DIR_TARGETS := $(SUBDIRS:%=build-dir-%)

export BUILD_DIR := $(CURDIR)/$(BUILD_DIR)
export CC
export CFLAGS
export LDFLAGS
export DEFINES

all: $(SUBDIRS)

clean:
	@Write-Output "Cleaning everything..."
	@if (Test-Path $(BUILD_DIR)) { Remove-Item -Recurse -Force $(BUILD_DIR) }

build-dir: $(BUILD_DIR) $(BUILD_DIR_TARGETS)

$(CLEAN_TARGETS):
	@$(MAKE) -C $(@:clean-%=%) -f Makefile.windows.mak clean

$(SUBDIRS):
	@$(MAKE) -C $@ -f Makefile.windows.mak
	@Write-Output "--------------------------------------------"

$(BUILD_DIR):
	@Write-Output "Creating directory $(BUILD_DIR)..."
	@if (!(Test-Path $(BUILD_DIR))) { New-Item -ItemType Directory -Path $(BUILD_DIR) | Out-Null }

$(BUILD_DIR_TARGETS):
	@$(MAKE) -C $(@:build-dir-%=%) -f Makefile.windows.mak build-dir

.PHONY: all clean build-dir $(SUBDIRS) $(CLEAN_TARGETS) $(BUILD_DIR_TARGETS)
