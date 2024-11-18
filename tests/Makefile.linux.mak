TARGET := tests
SRCDIR := src
SRCS := $(shell find $(SRCDIR) -type f -name "*.c")
CFLAGS +=
INCLUDES := -I ../engine/src
LDFLAGS += -L $(BUILD_DIR)/engine -lsnuk -Wl,-rpath=$(BUILD_DIR)/engine
DEFINES +=

OBJS := $(SRCS:%.c=$(BUILD_DIR)/tests/%.o)
DEPS := $(OBJS:.o=.d)
CFLAGS += -MMD -MP $(INCLUDES) $(DEFINES)
TARGET := $(BUILD_DIR)/tests/$(TARGET)
DIRS := $(sort $(dir $(OBJS)))

all: $(TARGET)

clean:
	@echo "Cleaning tests..."
	@rm -rf $(BUILD_DIR)/tests

$(TARGET): $(OBJS)
	@echo "Linking $@..."
	@$(CC) $^ -o $@ $(LDFLAGS)

$(BUILD_DIR)/tests/%.o: %.c | $(DIRS)
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

$(DIRS):
	@echo "Creating directory $@..."
	@mkdir -p $@

.PHONY: all clean

-include $(DEPS)