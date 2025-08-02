# Detect OS
ifeq ($(OS),Windows_NT)
    GENERATOR := "Visual Studio 17 2022"
    BUILD_CMD := cmake --build build --config Debug
    RUN_CMD := ./build/Debug/eternum-engine.exe
else
    ifeq (, $(shell which ninja))
        GENERATOR := "Unix Makefiles"
    else
        GENERATOR := Ninja
    endif
    BUILD_CMD := cmake --build build
    RUN_CMD := ./build/eternum-engine
endif

# Default target
all: configure build

# Configure step
configure:
	@mkdir -p build
	@cd build && cmake -G $(GENERATOR) ..

# Build step
build:
	$(BUILD_CMD)

# Run main executable
run:
	$(RUN_CMD)

# Run tests
test:
	@cd build && ctest --output-on-failure

# Clean build folder
clean:
	@rm -rf build

.PHONY: all configure build run test clean
