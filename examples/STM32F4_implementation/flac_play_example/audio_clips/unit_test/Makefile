TARGET_EXEC ?= unit_test.out

BUILD_DIR ?= ./build
SRC_DIRS ?= .. ./mocks

# my stuff
SRCS := $(shell find $(SRC_DIRS) -maxdepth 2 -name \*.cpp -or -name \*.c -or -name \*.s)
INC_DIRS := $(shell find $(SRC_DIRS) -maxdepth 2 -name "include" -type d)
INC_DIRS += $(SRC_DIRS)

# flac
FLAC_DIR := ../libfoxenflac/
SRCS += $(FLAC_DIR)/foxen/flac.c
INC_DIRS += $(shell find $(FLAC_DIR) -name libfoxen\* -type d) # subprojects

# objects and dependencies
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP -lm

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# assembly
$(BUILD_DIR)/%.s.o: %.s
	$(MKDIR_P) $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

# c source
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@


.PHONY: clean run

run:
	@./$(BUILD_DIR)/$(TARGET_EXEC)

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p
