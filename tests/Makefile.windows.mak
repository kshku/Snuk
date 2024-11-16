SHELL := powershell.exe
.SHELLFLAGS := -NoProfile -NoInteractive -Command

rwildcard = $(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

TARGET := tests.exe
SRCDIR := src
SRCS := $(call rwildcard,$(SRCDIR)/,*.c)
CFLAGS += -fdeclspec
INCLUDES := -I ../engine/src
LDFLAGS += -L $(BUILD_DIR)/engine -lsnuk
DEFINES +=

OBJS := $(SRCS:%.c=$(BUILD_DIR)/tests/%.o)
DEPS := $(OBJS:.o=.d)
CFLAGS += -MMD -MP $(INCLUDES) $(DEFINES)
TARGET := $(BUILD_DIR)/tests/$(TARGET)
DIRS := $(sort $(dir $(OBJS)))

all: $(DIRS) $(TARGET)

clean:
	@Write-Output "Cleaning tests..."
	@if (Test-Path $(BUILD_DIR)/tests) { Remove-Item -Recurse -Force $(BUILD_DIR)/tests }

$(TARGET): $(OBJS)
	@Write-Output "Linking $@..."
	@$(CC) $^ -o $@ $(LDFLAGS)

$(BUILD_DIR)/tests/%.o: %.c
	@Write-Output "Compiling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

$(DIRS):
	@Write-Output "Creating directory $@..."
	@if (!(Test-Path $@)) { New-Item -ItemType Directory -Path $@ | Out-Null }

.PHONY: all clean

-include $(DEPS)
