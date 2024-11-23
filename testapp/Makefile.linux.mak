TARGET := testapp
SRCDIR := src
SRCS := $(shell find $(SRCDIR) -type f -name "*.c")
CFLAGS +=
INCLUDES := -I ../engine/src
X11 := $(shell pkg-config --cflags --libs x11)
XCB := $(shell pkg-config --cflags --libs xcb xcb-icccm)
LIBRARIES := $(X11) $(XCB)
LDFLAGS += -L $(BUILD_DIR)/engine -lsnuk -Wl,-rpath=$(BUILD_DIR)/engine $(LIBRARIES)
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
