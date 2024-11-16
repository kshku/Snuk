SHELL := powershell.exe
.SHELLFLAGS := -NoProfile -NoInteractive -Command

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
	@Write-Output "Cleaning everything..."
	@if (Test-Path $(BUILD_DIR)) { Remove-Item -Recurse -Force $(BUILD_DIR) }

$(CLEAN_TARGETS):
	@$(MAKE) -C $(@:clean-%=%) -f Makefile.windows.mak clean

$(SUBDIRS): | $(BUILD_DIR)
	@$(MAKE) -C $@ -f Makefile.windows.mak
	@Write-Output "--------------------------------------------"

$(BUILD_DIR):
	@Write-Output "Creating directory $(BUILD_DIR)..."
	@if (!(Test-Path $(BUILD_DIR))) { New-Item -ItemType Directory -Path $(BUILD_DIR) | Out-Null }

.PHONY: all clean $(SUBDIRS) $(CLEAN_TARGETS)
