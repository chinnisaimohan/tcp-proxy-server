# Makefile for TCP Proxy Server Project
# Platform: Windows with MinGW-w64
# Compiler: g++ with C++17 support

# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
LDFLAGS = -lws2_32

# Directories
SRC_DIR = src
BIN_DIR = bin
CONFIG_DIR = config

# Target executables
PROXY_SERVER = $(BIN_DIR)/proxy_server.exe
TEST_COMPREHENSIVE = $(BIN_DIR)/test_comprehensive.exe
TEST_EDGE_CASES = $(BIN_DIR)/test_edge_cases.exe
TEST_STRESS = $(BIN_DIR)/test_stress.exe

# Source files
PROXY_SRC = $(SRC_DIR)/proxy_server.cpp
TEST_COMPREHENSIVE_SRC = $(SRC_DIR)/test_comprehensive.cpp
TEST_EDGE_CASES_SRC = $(SRC_DIR)/test_edge_cases.cpp
TEST_STRESS_SRC = $(SRC_DIR)/test_stress.cpp

# Default target: build all
.PHONY: all
all: $(PROXY_SERVER) $(TEST_COMPREHENSIVE) $(TEST_EDGE_CASES) $(TEST_STRESS)
	@echo ========================================
	@echo   BUILD COMPLETE
	@echo ========================================
	@echo All executables compiled successfully!

# Build proxy server
$(PROXY_SERVER): $(PROXY_SRC)
	@echo Building proxy server...
	@if not exist $(BIN_DIR) mkdir $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(PROXY_SRC) -o $(PROXY_SERVER) $(LDFLAGS)
	@echo [OK] proxy_server.exe

# Build comprehensive test client
$(TEST_COMPREHENSIVE): $(TEST_COMPREHENSIVE_SRC)
	@echo Building comprehensive test client...
	$(CXX) $(CXXFLAGS) $(TEST_COMPREHENSIVE_SRC) -o $(TEST_COMPREHENSIVE) $(LDFLAGS)
	@echo [OK] test_comprehensive.exe

# Build edge cases test client
$(TEST_EDGE_CASES): $(TEST_EDGE_CASES_SRC)
	@echo Building edge cases test client...
	$(CXX) $(CXXFLAGS) $(TEST_EDGE_CASES_SRC) -o $(TEST_EDGE_CASES) $(LDFLAGS)
	@echo [OK] test_edge_cases.exe

# Build stress test client
$(TEST_STRESS): $(TEST_STRESS_SRC)
	@echo Building stress test client...
	$(CXX) $(CXXFLAGS) $(TEST_STRESS_SRC) -o $(TEST_STRESS) $(LDFLAGS)
	@echo [OK] test_stress.exe

# Build only the proxy server
.PHONY: server
server: $(PROXY_SERVER)

# Build only test clients
.PHONY: tests
tests: $(TEST_COMPREHENSIVE) $(TEST_EDGE_CASES) $(TEST_STRESS)

# Run the proxy server
.PHONY: run
run: $(PROXY_SERVER)
	@echo Starting proxy server on port 8888...
	@$(PROXY_SERVER)

# Run all test suites
.PHONY: test
test: all
	@echo ========================================
	@echo   RUNNING TEST SUITES
	@echo ========================================
	@powershell -Command ".\test_suite.ps1"
	@powershell -Command ".\test_post_bodies.ps1"
	@$(TEST_STRESS)

# Clean build artifacts
.PHONY: clean
clean:
	@echo Cleaning build artifacts...
	@if exist $(BIN_DIR)\*.exe del /Q $(BIN_DIR)\*.exe
	@if exist proxy.log del /Q proxy.log
	@echo Clean complete!

# Clean everything including logs
.PHONY: distclean
distclean: clean
	@if exist $(BIN_DIR) rmdir /S /Q $(BIN_DIR)
	@echo Deep clean complete!

# Install (copy to system location - optional)
.PHONY: install
install: all
	@echo Installation not implemented (Windows manual installation)
	@echo Copy bin\proxy_server.exe to desired location

# Show help
.PHONY: help
help:
	@echo ========================================
	@echo   TCP Proxy Server - Makefile Help
	@echo ========================================
	@echo Available targets:
	@echo   make all        - Build all executables (default)
	@echo   make server     - Build only proxy server
	@echo   make tests      - Build only test clients
	@echo   make run        - Build and run proxy server
	@echo   make test       - Build all and run test suites
	@echo   make clean      - Remove compiled executables
	@echo   make distclean  - Remove all build artifacts
	@echo   make help       - Show this help message
	@echo Examples:
	@echo   make            - Build everything
	@echo   make clean all  - Clean then rebuild
	@echo   make run        - Start the proxy server
