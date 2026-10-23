BUILD_DIR ?= build
CMAKE ?= cmake

.PHONY: all build clean run help

all: build

build:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && $(CMAKE) .. && $(CMAKE) --build . -j$$(nproc)
	@cp -f $(BUILD_DIR)/pixel-forge . 2>/dev/null || true

clean:
	@rm -rf $(BUILD_DIR) pixel-forge output*.png output*.gif output*.json

run: build
	@./pixel-forge

help:
	@echo "PixelForge AI build targets:"
	@echo "  make         - Build pixel-forge"
	@echo "  make run     - Run interactive pixel-forge CLI"
	@echo "  make clean   - Clean build artifacts"
