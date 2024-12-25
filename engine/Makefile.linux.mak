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

# Protocols xml for wayland
WAYLAND_PROTO_DIR := $(SRCDIR)/platform/windowing/wayland/protocols
WAYLAND_PROTO_XML := $(shell find $(WAYLAND_PROTO_DIR) -type f -name "*.xml")
WAYLAND_PROTO_HEADER := $(WAYLAND_PROTO_XML:%.xml=%.h)
WAYLAND_PROTO_CODE := $(WAYLAND_PROTO_XML:%.xml=%.c)

all: wayland-protocols $(TARGET)

clean:
	@echo "Cleaning engine..."
	@rm -rf $(BUILD_DIR)/engine

wayland-protocols: $(WAYLAND_PROTO_HEADER) $(WAYLAND_PROTO_CODE)

clean-wayland-protocols:
	@echo "Removing the generated protocol codes"
	@rm -f $(WAYLAND_PROTO_CODE) $(WAYLAND_PROTO_HEADER)

$(TARGET): $(OBJS)
	@echo "Linking $@..."
	@$(CC) $(LDFLAGS) $^ -o $@

$(BUILD_DIR)/engine/%.o: %.c | $(DIRS)
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

$(DIRS):
	@echo "Creating directory $@..."
	@mkdir -p $@

$(WAYLAND_PROTO_DIR)/%.h: $(WAYLAND_PROTO_DIR)/%.xml
	@echo "Generating the protocol client header $@ from $<..."
	@wayland-scanner client-header $< $@

$(WAYLAND_PROTO_DIR)/%.c: $(WAYLAND_PROTO_DIR)/%.xml
	@echo "Generating the protocol private code $@ from $<..."
	@wayland-scanner private-code $< $@

.PHONY: all clean wayland-protocols clean-wayland-protocols

-include $(DEPS)
