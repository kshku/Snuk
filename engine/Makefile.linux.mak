TARGET := libsnuk.so
SRCDIR := src
SRCS := $(shell find $(SRCDIR) -type f -name "*.c")
CFLAGS += -fvisibility=hidden -fPIC
XLIB := x11 xi
XCB := xcb xcb-icccm x11-xcb xcb-xinput
WAYLAND := wayland-client
INCLUDES := -I $(SRCDIR)
INCLUDES += $(shell pkg-config --cflags $(XLIB) $(XCB) $(WAYLAND))
LDFLAGS += -shared
LDFLAGS += $(shell pkg-config --libs $(XLIB) $(XCB) $(WAYLAND))
DEFINES += -DS_EXPORTS

OBJS := $(SRCS:%.c=$(BUILD_DIR)/engine/%.o)
DEPS := $(OBJS:.o=.d)
CFLAGS += -MMD -MP $(INCLUDES) $(DEFINES)
TARGET := $(BUILD_DIR)/engine/$(TARGET)
DIRS := $(sort $(dir $(OBJS)))

all: $(TARGET)

clean:
	@echo "Cleaning engine..."
	@rm -rf $(BUILD_DIR)/engine

$(TARGET): $(OBJS)
	@echo "Linking $@..."
	@$(CC) $(LDFLAGS) $^ -o $@

$(BUILD_DIR)/engine/%.o: %.c | $(DIRS)
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

$(DIRS):
	@echo "Creating directory $@..."
	@mkdir -p $@

.PHONY: all clean

-include $(DEPS)
