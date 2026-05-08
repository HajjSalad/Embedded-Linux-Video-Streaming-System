# ========================================================
# Makefile for Raspberry Pi: Kernel module + User program
# ========================================================

# 0. Reminder of build flow
# Source files (.c/.cpp) are compiled into object files (.o)
# Object files are linked to produce a final executable
# Kernel modules (.c) are compiled into .ko files using the kernel build system
# User program runs on terminal
# --------------------------------------------------------

# 1. Kernel Module configuration
KDIR := /lib/modules/$(shell uname -r)/build

# 2. Compilers
CC  := gcc
CXX := g++

# 3. User Program configuration
USER_PROG := camera_client
BUILD_DIR := build
SRC_DIR   := src

# Include paths
USER_INC := -I$(CURDIR)/src -I$(CURDIR)/kernel

# Compiler flags
USER_CFLAGS   := -std=c11 -Wall -Wextra -O2
USER_CXXFLAGS := -std=c++17 -Wall -Wextra -O2

# Libraries
USER_LIBS   += -ltensorflow-lite -ljpeg

# 4. Source files

# Recursively find all C/C++ source files
USER_C_SRC   := $(shell find $(SRC_DIR) -name "*.c")
USER_CPP_SRC := $(shell find $(SRC_DIR) -name "*.cpp")

# 5. Object files
# Map source files to object files in the BUILD_DIR
USER_OBJ := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(USER_C_SRC)) \
            $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(USER_CPP_SRC))

# 6. Default target
all: module user

# 7. Build kernel module
module:
	$(MAKE) -C $(KDIR) M=$(CURDIR)/kernel modules

# 8. Build user-space program
user: $(USER_OBJ)
	@echo "Linking user-space program..."
	$(CXX) $(USER_OBJ) -o $(USER_PROG) $(USER_LIBS)

# 9. Compile rules
# C source compilation
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(USER_CFLAGS) $(USER_INC) -c $< -o $@

# C++ source compilation
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(USER_CXXFLAGS) $(USER_INC) -c $< -o $@

# 10. Clean target
clean:
	$(MAKE) -C $(KDIR) M=$(CURDIR)/kernel clean
	rm -rf $(BUILD_DIR) $(USER_PROG)

.PHONY: all module user clean
