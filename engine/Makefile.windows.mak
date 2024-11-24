SHELL := powershell.exe
.SHELLFLAGS := -NoProfile -NoInteractive -Command

rwildcard = $(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

TARGET := snuk.dll
SRCDIR := src
SRCS := $(call rwildcard,$(SRCDIR)/,*.c)
CFLAGS += -fdeclspec
INCLUDES := -I $(SRCDIR)
LDFLAGS += -shared -luser32
DEFINES += -DS_EXPORTS

OBJS := $(SRCS:%.c=$(BUILD_DIR)/engine/%.o)
DEPS := $(OBJS:.o=.d)
CFLAGS += -MMD -MP $(INCLUDES) $(DEFINES)
TARGET := $(BUILD_DIR)/engine/$(TARGET)
DIRS := $(sort $(dir $(OBJS)))

all: $(DIRS) $(TARGET)

clean:
	@Write-Output "Cleaning engine..."
	@if (Test-Path $(BUILD_DIR)/engine) { Remove-Item -Recurse -Force $(BUILD_DIR)/engine }

$(TARGET): $(OBJS)
	@Write-Output "Linking $@..."
	@$(CC) $(LDFLAGS) $^ -o $@

$(BUILD_DIR)/engine/%.o: %.c
	@Write-Output "Compiling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

$(DIRS):
	@Write-Output "Creating directory $@..."
	@if (!(Test-Path $@)) { New-Item -ItemType Directory -Path $@ | Out-Null }

.PHONY: all clean

-include $(DEPS)
