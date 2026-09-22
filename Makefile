BUILD_DIR ?= build

.PHONY: all clean help

all:
	@cmake -B $(BUILD_DIR)
	@cmake --build $(BUILD_DIR)

clean:
	@rm -rf $(BUILD_DIR)
	@echo "Cleaned $(BUILD_DIR) directory."

help:
	@echo "Build commands:"
	@echo "  make        - Configure and build via CMake"
	@echo "  make clean  - Remove build directory"
