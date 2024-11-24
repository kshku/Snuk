TARGET := testapp
SRCDIR := src
SRCS := $(shell find $(SRCDIR) -type f -name "*.c")
CFLAGS +=
XLIB := x11
XCB := xcb xcb-icccm
WAYLAND := wayland-client
INCLUDES := -I ../engine/src
INCLUDES += $(shell pkg-config --cflags $(XLIB) $(XCB) $(WAYLAND))
LDFLAGS += -L $(BUILD_DIR)/engine -lsnuk -Wl,-rpath=$(BUILD_DIR)/engine
LDFLAGS += $(shell pkg-config --libs $(XLIB) $(XCB) $(WAYLAND))
DEFINES +=

OBJS := $(SRCS:%.c=$(BUILD_DIR)/testapp/%.o)
DEPS := $(OBJS:.o=.d)
CFLAGS += -MMD -MP $(INCLUDES) $(DEFINES)
TARGET := $(BUILD_DIR)/testapp/$(TARGET)
DIRS := $(sort $(dir $(OBJS)))

all: $(TARGET)

clean:
	@echo "Cleaning testapp..."
	@rm -rf $(BUILD_DIR)/testapp

$(TARGET): $(OBJS)
	@echo "Linking $@..."
	@$(CC) $^ -o $@ $(LDFLAGS)

$(BUILD_DIR)/testapp/%.o: %.c | $(DIRS)
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

$(DIRS):
	@echo "Creating directory $@..."
	@mkdir -p $@

.PHONY: all clean

-include $(DEPS)
