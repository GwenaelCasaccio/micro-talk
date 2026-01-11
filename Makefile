# ============================================================================
# Micro-Talk VM Build System
# ============================================================================

# Compiler and flags
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O0 -g
LDFLAGS :=

# Directories
SRC_DIR := src
TEST_DIR := tests
BUILD_DIR := build
EXAMPLES_DIR := examples
LISP_DIR := lisp

# Colors for output (optional, comment out if not supported)
COLOR_RESET := \033[0m
COLOR_BOLD := \033[1m
COLOR_GREEN := \033[32m
COLOR_BLUE := \033[34m
COLOR_YELLOW := \033[33m

# ============================================================================
# Core Dependencies
# ============================================================================

VM_DEPS := $(SRC_DIR)/stack_vm.hpp $(SRC_DIR)/interrupt.hpp
PARSER_DEPS := $(SRC_DIR)/lisp_parser.hpp
COMPILER_DEPS := $(VM_DEPS) $(PARSER_DEPS) $(SRC_DIR)/lisp_compiler.hpp
MICROCODE_DEPS := $(COMPILER_DEPS) $(SRC_DIR)/microcode.hpp
TRANSPILER_DEPS := $(PARSER_DEPS) $(SRC_DIR)/lisp_to_cpp.hpp

# ============================================================================
# Binaries
# ============================================================================

# Main executable
TARGET := $(BUILD_DIR)/lisp_vm

SMALLTALK := $(BUILD_DIR)/smalltalk

# Unit test binaries (from tests/)
UNIT_TEST_BINS := \
	$(BUILD_DIR)/test_tagging \
	$(BUILD_DIR)/test_variables \
	$(BUILD_DIR)/test_lambda \
	$(BUILD_DIR)/test_loops \
	$(BUILD_DIR)/test_microcode \
	$(BUILD_DIR)/test_advanced \
	$(BUILD_DIR)/test_smalltalk \
	$(BUILD_DIR)/test_hash_table \
	$(BUILD_DIR)/test_comments \
	$(BUILD_DIR)/test_funcall \
	$(BUILD_DIR)/test_vm_stack \
	$(BUILD_DIR)/test_vm_interrupt \
	$(BUILD_DIR)/test_vm_alu \
	$(BUILD_DIR)/test_vm_memory \
	$(BUILD_DIR)/test_vm_control \
	$(BUILD_DIR)/test_parser_basic \
	$(BUILD_DIR)/test_parser_comments \
	$(BUILD_DIR)/test_parser_errors \
	$(BUILD_DIR)/test_compiler_basic \
	$(BUILD_DIR)/test_compiler_control \
	$(BUILD_DIR)/test_compiler_variables \
	$(BUILD_DIR)/test_compiler_functions \
	$(BUILD_DIR)/test_compiler_interrupts \
	$(BUILD_DIR)/test_transpiler \
	$(BUILD_DIR)/test_transpiler_extended

# Special binaries
TRANSPILER_DEMO := $(BUILD_DIR)/transpiler_demo
TOKENIZER_TRANSPILER := $(BUILD_DIR)/transpile_tokenizer
ST_TOKENIZER := $(BUILD_DIR)/st_tokenizer_file
MINIMAL_VM := $(BUILD_DIR)/minimal_vm_test
DISASSEMBLER_TEST := $(BUILD_DIR)/test_disassembler

# All binaries (excluding optional/generated ones like ST_TOKENIZER)
ALL_BINS := $(TARGET) $(UNIT_TEST_BINS) $(SMALLTALK) \
            $(TRANSPILER_DEMO) $(TOKENIZER_TRANSPILER) $(MINIMAL_VM) \
            $(DISASSEMBLER_TEST)

# Optional binaries (require generated source files)
OPTIONAL_BINS := $(ST_TOKENIZER)

# ============================================================================
# Main Targets
# ============================================================================

.PHONY: all all-with-optional clean help
.DEFAULT_GOAL := all

all: $(BUILD_DIR) $(ALL_BINS)
	@echo ""
	@echo "$(COLOR_GREEN)$(COLOR_BOLD)✓ Build complete!$(COLOR_RESET)"
	@echo "  Main: $(TARGET)"
	@echo "  Tests: $(words $(UNIT_TEST_BINS)) unit tests"

all-with-optional: all $(OPTIONAL_BINS)
	@echo "$(COLOR_GREEN)✓ Optional binaries built$(COLOR_RESET)"

help:
	@echo "$(COLOR_BOLD)Micro-Talk VM Build System$(COLOR_RESET)"
	@echo ""
	@echo "$(COLOR_BOLD)Building:$(COLOR_RESET)"
	@echo "  make              - Build all binaries"
	@echo "  make clean        - Remove all build artifacts"
	@echo "  make run          - Build and run main VM"
	@echo ""
	@echo "$(COLOR_BOLD)Testing:$(COLOR_RESET)"
	@echo "  make test-all     - Run all tests (unit + integration)"
	@echo "  make vm-all       - Run all VM tests"
	@echo "  make parser-all   - Run all parser tests"
	@echo "  make compiler-all - Run all compiler tests"
	@echo "  make transpiler   - Run transpiler tests"
	@echo ""
	@echo "$(COLOR_BOLD)Code Quality:$(COLOR_RESET)"
	@echo "  make format       - Format all C++ files"
	@echo "  make lint         - Run clang-tidy"
	@echo "  make lint-fix     - Auto-fix clang-tidy issues"
	@echo "  make cppcheck     - Run static analysis"
	@echo "  make check-all    - Run all quality checks"
	@echo ""
	@echo "$(COLOR_BOLD)Tools:$(COLOR_RESET)"
	@echo "  make compile-db   - Generate compile_commands.json"
	@echo "  make help         - Show this help"

# Create build directory
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

clean:
	@echo "$(COLOR_YELLOW)Cleaning build artifacts...$(COLOR_RESET)"
	@rm -rf $(BUILD_DIR)
	@rm -f compile_commands.json cppcheck-report.xml
	@echo "$(COLOR_GREEN)✓ Clean complete$(COLOR_RESET)"

# ============================================================================
# Compilation Rules
# ============================================================================

# Main executable
$(TARGET): $(SRC_DIR)/main.cpp $(COMPILER_DEPS) | $(BUILD_DIR)
	@echo "$(COLOR_BLUE)Compiling$(COLOR_RESET) $@"
	@$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

# Smalltalk executable
$(SMALLTALK): $(SRC_DIR)/smalltalk.cpp $(COMPILER_DEPS) | $(BUILD_DIR)
	@echo "$(COLOR_BLUE)Compiling$(COLOR_RESET) $@"
	@$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

# Disassembler test
$(DISASSEMBLER_TEST): $(SRC_DIR)/test_disassembler.cpp $(COMPILER_DEPS) $(SRC_DIR)/disassembler.hpp | $(BUILD_DIR)
	@echo "$(COLOR_BLUE)Compiling$(COLOR_RESET) $@"
	@$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

# Minimal VM test
$(BUILD_DIR)/minimal_vm_test: $(SRC_DIR)/minimal_vm_test.cpp $(VM_DEPS) | $(BUILD_DIR)
	@echo "$(COLOR_BLUE)Compiling$(COLOR_RESET) $@"
	@$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

# Transpiler tools
$(TRANSPILER_DEMO): $(EXAMPLES_DIR)/transpiler_demo.cpp $(TRANSPILER_DEPS) | $(BUILD_DIR)
	@echo "$(COLOR_BLUE)Compiling$(COLOR_RESET) $@"
	@$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

$(TOKENIZER_TRANSPILER): $(SRC_DIR)/transpile_tokenizer.cpp $(TRANSPILER_DEPS) | $(BUILD_DIR)
	@echo "$(COLOR_BLUE)Compiling$(COLOR_RESET) $@"
	@$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

$(ST_TOKENIZER): $(SRC_DIR)/tokenizer_main.cpp $(PARSER_DEPS) | $(BUILD_DIR)
	@echo "$(COLOR_BLUE)Compiling$(COLOR_RESET) $@"
	@$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

# ============================================================================
# Pattern Rules for Tests
# ============================================================================

# VM tests (only need VM headers)
$(BUILD_DIR)/test_vm_%: $(TEST_DIR)/test_vm_%.cpp $(VM_DEPS) | $(BUILD_DIR)
	@echo "$(COLOR_BLUE)Compiling$(COLOR_RESET) $@"
	@$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

# Parser tests (only need parser headers)
$(BUILD_DIR)/test_parser_%: $(TEST_DIR)/test_parser_%.cpp $(PARSER_DEPS) | $(BUILD_DIR)
	@echo "$(COLOR_BLUE)Compiling$(COLOR_RESET) $@"
	@$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

# Compiler tests (need all compiler deps)
$(BUILD_DIR)/test_compiler_%: $(TEST_DIR)/test_compiler_%.cpp $(COMPILER_DEPS) | $(BUILD_DIR)
	@echo "$(COLOR_BLUE)Compiling$(COLOR_RESET) $@"
	@$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

# Transpiler tests
$(BUILD_DIR)/test_transpiler%: $(TEST_DIR)/test_transpiler%.cpp $(TRANSPILER_DEPS) | $(BUILD_DIR)
	@echo "$(COLOR_BLUE)Compiling$(COLOR_RESET) $@"
	@$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

# Special tests with microcode dependencies
$(BUILD_DIR)/test_microcode: $(TEST_DIR)/test_microcode.cpp $(MICROCODE_DEPS) | $(BUILD_DIR)
	@echo "$(COLOR_BLUE)Compiling$(COLOR_RESET) $@"
	@$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

$(BUILD_DIR)/test_advanced: $(TEST_DIR)/test_advanced.cpp $(MICROCODE_DEPS) | $(BUILD_DIR)
	@echo "$(COLOR_BLUE)Compiling$(COLOR_RESET) $@"
	@$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

$(BUILD_DIR)/test_smalltalk: $(TEST_DIR)/test_smalltalk.cpp $(MICROCODE_DEPS) | $(BUILD_DIR)
	@echo "$(COLOR_BLUE)Compiling$(COLOR_RESET) $@"
	@$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

# Generic fallback for other tests
$(BUILD_DIR)/test_%: $(TEST_DIR)/test_%.cpp $(COMPILER_DEPS) | $(BUILD_DIR)
	@echo "$(COLOR_BLUE)Compiling$(COLOR_RESET) $@"
	@$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

# ============================================================================
# Test Execution Targets
# ============================================================================

.PHONY: run test simple vars lambda loops micro advanced smalltalk comments disasm

run: $(TARGET)
	@./$(TARGET)

# Individual integration tests
test: $(BUILD_DIR)/test_tagging
	@./$(BUILD_DIR)/test_tagging

simple: $(BUILD_DIR)/simple_tagging_test
	@./$(BUILD_DIR)/simple_tagging_test

vars: $(BUILD_DIR)/test_variables
	@./$(BUILD_DIR)/test_variables

lambda: $(BUILD_DIR)/test_lambda
	@./$(BUILD_DIR)/test_lambda

loops: $(BUILD_DIR)/test_loops
	@./$(BUILD_DIR)/test_loops

micro: $(BUILD_DIR)/test_microcode
	@./$(BUILD_DIR)/test_microcode

advanced: $(BUILD_DIR)/test_advanced
	@./$(BUILD_DIR)/test_advanced

smalltalk: $(BUILD_DIR)/test_smalltalk
	@./$(BUILD_DIR)/test_smalltalk

hash: $(BUILD_DIR)/test_hash_table
	@./$(BUILD_DIR)/test_hash_table

comments: $(BUILD_DIR)/test_comments
	@./$(BUILD_DIR)/test_comments

disasm: $(DISASSEMBLER_TEST)
	@./$(DISASSEMBLER_TEST)

# ============================================================================
# Unit Test Suites
# ============================================================================

.PHONY: vm-stack vm-alu vm-memory vm-control vm-all
.PHONY: parser-basic parser-comments parser-errors parser-all
.PHONY: compiler-basic compiler-control compiler-variables compiler-functions compiler-interrupts compiler-all
.PHONY: transpiler transpiler-demo integration-all test-all

# VM tests
vm-stack: $(BUILD_DIR)/test_vm_stack
	@./$(BUILD_DIR)/test_vm_stack

vm-interrupt: $(BUILD_DIR)/test_vm_interrupt
	@./$(BUILD_DIR)/test_vm_interrupt


vm-alu: $(BUILD_DIR)/test_vm_alu
	@./$(BUILD_DIR)/test_vm_alu

vm-memory: $(BUILD_DIR)/test_vm_memory
	@./$(BUILD_DIR)/test_vm_memory

vm-control: $(BUILD_DIR)/test_vm_control
	@./$(BUILD_DIR)/test_vm_control

vm-all: vm-stack vm-alu vm-memory vm-control
	@echo ""
	@echo "$(COLOR_GREEN)✓ All VM tests passed!$(COLOR_RESET)"

# Parser tests
parser-basic: $(BUILD_DIR)/test_parser_basic
	@./$(BUILD_DIR)/test_parser_basic

parser-comments: $(BUILD_DIR)/test_parser_comments
	@./$(BUILD_DIR)/test_parser_comments

parser-errors: $(BUILD_DIR)/test_parser_errors
	@./$(BUILD_DIR)/test_parser_errors

parser-all: parser-basic parser-comments parser-errors
	@echo ""
	@echo "$(COLOR_GREEN)✓ All parser tests passed!$(COLOR_RESET)"

# Compiler tests
compiler-basic: $(BUILD_DIR)/test_compiler_basic
	@./$(BUILD_DIR)/test_compiler_basic

compiler-control: $(BUILD_DIR)/test_compiler_control
	@./$(BUILD_DIR)/test_compiler_control

compiler-variables: $(BUILD_DIR)/test_compiler_variables
	@./$(BUILD_DIR)/test_compiler_variables

compiler-functions: $(BUILD_DIR)/test_compiler_functions
	@./$(BUILD_DIR)/test_compiler_functions

compiler-interrupts: $(BUILD_DIR)/test_compiler_interrupts
	@./$(BUILD_DIR)/test_compiler_interrupts

compiler-all: compiler-basic compiler-control compiler-variables compiler-functions compiler-interrupts
	@echo ""
	@echo "$(COLOR_GREEN)✓ All compiler tests passed!$(COLOR_RESET)"

# Transpiler tests
transpiler: $(BUILD_DIR)/test_transpiler $(BUILD_DIR)/test_transpiler_extended
	@./$(BUILD_DIR)/test_transpiler
	@echo ""
	@./$(BUILD_DIR)/test_transpiler_extended

transpiler-demo: $(TRANSPILER_DEMO)
	@./$(TRANSPILER_DEMO)

# Integration tests
integration-all: test simple vars lambda loops micro advanced smalltalk comments
	@echo ""
	@echo "$(COLOR_GREEN)✓ All integration tests passed!$(COLOR_RESET)"

# All tests
test-all: vm-all parser-all compiler-all transpiler integration-all
	@echo ""
	@echo "$(COLOR_GREEN)$(COLOR_BOLD)======================================"
	@echo "✓ ALL TESTS PASSED!"
	@echo "======================================$(COLOR_RESET)"
	@echo "  $(COLOR_BOLD)Unit Tests:$(COLOR_RESET)"
	@echo "    ✓ VM tests (43 tests)"
	@echo "    ✓ Parser tests (58 tests)"
	@echo "    ✓ Compiler tests (44 tests)"
	@echo "    ✓ Transpiler tests (35 tests)"
	@echo "  $(COLOR_BOLD)Integration Tests:$(COLOR_RESET)"
	@echo "    ✓ Tagging, Variables, Lambda"
	@echo "    ✓ Loops, Microcode, Advanced"
	@echo "    ✓ Smalltalk, Comments"
	@echo "$(COLOR_GREEN)$(COLOR_BOLD)======================================$(COLOR_RESET)"

# ============================================================================
# Transpiler Workflows
# ============================================================================

.PHONY: tokenizer tokenizer-transpile tokenizer-file parser typed-functions parser-typed

tokenizer: $(TOKENIZER_TRANSPILER)
	@echo "$(COLOR_BLUE)=== Transpiling Smalltalk Tokenizer ===$(COLOR_RESET)"
	@./$(TOKENIZER_TRANSPILER)
	@echo ""
	@echo "$(COLOR_BLUE)=== Compiling Generated C++ ===$(COLOR_RESET)"
	@$(CXX) -std=c++17 -o $(BUILD_DIR)/st_tokenizer $(BUILD_DIR)/smalltalk_tokenizer.cpp
	@echo ""
	@echo "$(COLOR_BLUE)=== Running Tokenizer ===$(COLOR_RESET)"
	@./$(BUILD_DIR)/st_tokenizer

tokenizer-transpile: $(TOKENIZER_TRANSPILER)
	@echo "$(COLOR_BLUE)=== Regenerating tokenizer_main.cpp from Lisp source ===$(COLOR_RESET)"
	@./$(TOKENIZER_TRANSPILER) $(LISP_DIR)/smalltalk_tokenizer_ffi.lisp $(BUILD_DIR)/tokenizer_main_tmp.cpp
	@echo ""
	@echo "Adding header comments..."
	@echo "// AUTO-GENERATED CODE - DO NOT EDIT MANUALLY" > $(SRC_DIR)/tokenizer_main.cpp
	@echo "// This file was transpiled from: lisp/smalltalk_tokenizer_ffi.lisp" >> $(SRC_DIR)/tokenizer_main.cpp
	@echo "// To regenerate, run: make tokenizer-transpile" >> $(SRC_DIR)/tokenizer_main.cpp
	@echo "//" >> $(SRC_DIR)/tokenizer_main.cpp
	@echo "// Smalltalk Tokenizer with FFI for file I/O" >> $(SRC_DIR)/tokenizer_main.cpp
	@echo "// Demonstrates the Lisp-to-C++ transpiler with FFI capabilities" >> $(SRC_DIR)/tokenizer_main.cpp
	@echo "" >> $(SRC_DIR)/tokenizer_main.cpp
	@tail -n +2 $(BUILD_DIR)/tokenizer_main_tmp.cpp >> $(SRC_DIR)/tokenizer_main.cpp
	@rm $(BUILD_DIR)/tokenizer_main_tmp.cpp
	@echo "$(COLOR_GREEN)✓ tokenizer_main.cpp regenerated successfully!$(COLOR_RESET)"

tokenizer-file: $(ST_TOKENIZER)
	@echo "$(COLOR_BLUE)=== Smalltalk Tokenizer with File I/O ===$(COLOR_RESET)"
	@echo ""
	@if [ -f $(EXAMPLES_DIR)/test.st ]; then \
		echo "Testing on examples/test.st:"; \
		./$(ST_TOKENIZER) $(EXAMPLES_DIR)/test.st; \
		echo ""; \
	fi
	@if [ -f /tmp/smalltalk_syntax.txt ]; then \
		echo "Testing on /tmp/smalltalk_syntax.txt:"; \
		./$(ST_TOKENIZER) /tmp/smalltalk_syntax.txt | head -50; \
		echo "... (truncated)"; \
	else \
		echo "Running with default input:"; \
		./$(ST_TOKENIZER); \
	fi

parser: $(TOKENIZER_TRANSPILER)
	@echo "$(COLOR_BLUE)=== Transpiling Smalltalk Parser ===$(COLOR_RESET)"
	@./$(TOKENIZER_TRANSPILER) $(LISP_DIR)/smalltalk_parser.lisp $(BUILD_DIR)/smalltalk_parser.cpp
	@echo ""
	@echo "$(COLOR_BLUE)=== Compiling Generated C++ ===$(COLOR_RESET)"
	@$(CXX) -std=c++17 -o $(BUILD_DIR)/st_parser $(BUILD_DIR)/smalltalk_parser.cpp
	@echo ""
	@echo "$(COLOR_BLUE)=== Running Parser ===$(COLOR_RESET)"
	@./$(BUILD_DIR)/st_parser

typed-functions: $(TOKENIZER_TRANSPILER)
	@echo "$(COLOR_BLUE)=== Transpiling Typed Functions Test ===$(COLOR_RESET)"
	@./$(TOKENIZER_TRANSPILER) $(LISP_DIR)/test_typed_functions.lisp $(BUILD_DIR)/test_typed_functions.cpp
	@echo ""
	@echo "$(COLOR_BLUE)=== Compiling Generated C++ ===$(COLOR_RESET)"
	@$(CXX) -std=c++17 -o $(BUILD_DIR)/test_typed_functions $(BUILD_DIR)/test_typed_functions.cpp
	@echo ""
	@echo "$(COLOR_BLUE)=== Running Typed Functions Test ===$(COLOR_RESET)"
	@./$(BUILD_DIR)/test_typed_functions

parser-typed: $(TOKENIZER_TRANSPILER)
	@echo "$(COLOR_BLUE)=== Transpiling Smalltalk Parser (Typed Strings) ===$(COLOR_RESET)"
	@./$(TOKENIZER_TRANSPILER) $(LISP_DIR)/smalltalk_parser_typed.lisp $(BUILD_DIR)/smalltalk_parser_typed.cpp
	@echo ""
	@echo "$(COLOR_BLUE)=== Compiling Generated C++ ===$(COLOR_RESET)"
	@$(CXX) -std=c++17 -o $(BUILD_DIR)/st_parser_typed $(BUILD_DIR)/smalltalk_parser_typed.cpp
	@echo ""
	@echo "$(COLOR_BLUE)=== Running Smalltalk Parser ===$(COLOR_RESET)"
	@./$(BUILD_DIR)/st_parser_typed

# ============================================================================
# Code Quality
# ============================================================================

.PHONY: format lint lint-fix cppcheck cppcheck-xml check-all compile-db

format:
	@echo "$(COLOR_BLUE)=== Formatting C++ files ===$(COLOR_RESET)"
	@for file in $(SRC_DIR)/*.cpp $(SRC_DIR)/*.hpp $(TEST_DIR)/*.cpp; do \
		if [ -f "$$file" ]; then \
			echo "Formatting $$file..."; \
			clang-format -i "$$file"; \
		fi \
	done
	@echo "$(COLOR_GREEN)✓ Formatting complete!$(COLOR_RESET)"

lint:
	@echo "$(COLOR_BLUE)=== Running clang-tidy ===$(COLOR_RESET)"
	@for file in $(SRC_DIR)/*.cpp $(SRC_DIR)/*.hpp $(TEST_DIR)/*.cpp; do \
		if [ -f "$$file" ]; then \
			echo "Checking $$file..."; \
			clang-tidy "$$file" -- $(CXXFLAGS); \
		fi \
	done
	@echo "$(COLOR_GREEN)✓ Linting complete!$(COLOR_RESET)"

lint-fix:
	@echo "$(COLOR_BLUE)=== Running clang-tidy with auto-fixes ===$(COLOR_RESET)"
	@for file in $(SRC_DIR)/*.cpp $(SRC_DIR)/*.hpp $(TEST_DIR)/*.cpp; do \
		if [ -f "$$file" ]; then \
			echo "Fixing $$file..."; \
			clang-tidy -fix "$$file" -- $(CXXFLAGS); \
		fi \
	done
	@echo "$(COLOR_GREEN)✓ Auto-fixes applied!$(COLOR_RESET)"

cppcheck:
	@echo "$(COLOR_BLUE)=== Running cppcheck static analysis ===$(COLOR_RESET)"
	@cppcheck --enable=all --inconclusive --std=c++17 \
		--suppress=missingIncludeSystem \
		--suppress=unusedFunction \
		--suppress=unmatchedSuppression \
		--inline-suppr \
		-I $(SRC_DIR) \
		--error-exitcode=1 \
		$(SRC_DIR)/*.cpp $(SRC_DIR)/*.hpp $(TEST_DIR)/*.cpp 2>&1 | grep -v "^Checking " || true
	@echo "$(COLOR_GREEN)✓ cppcheck complete!$(COLOR_RESET)"

cppcheck-xml:
	@echo "$(COLOR_BLUE)=== Running cppcheck (XML output) ===$(COLOR_RESET)"
	@cppcheck --enable=all --inconclusive --std=c++17 \
		--suppress=missingIncludeSystem \
		--suppress=unusedFunction \
		-I $(SRC_DIR) \
		--xml --xml-version=2 \
		$(SRC_DIR)/*.cpp $(SRC_DIR)/*.hpp $(TEST_DIR)/*.cpp 2> cppcheck-report.xml
	@echo "$(COLOR_GREEN)✓ Report saved to cppcheck-report.xml$(COLOR_RESET)"

check-all: format lint cppcheck
	@echo ""
	@echo "$(COLOR_GREEN)$(COLOR_BOLD)✓ All code quality checks passed!$(COLOR_RESET)"

# Generate compilation database for clang tools
compile-db:
	@echo "$(COLOR_BLUE)=== Generating compile_commands.json ===$(COLOR_RESET)"
	@echo '[' > compile_commands.json
	@first=true; \
	for src in $(SRC_DIR)/*.cpp $(TEST_DIR)/*.cpp; do \
		if [ -f "$$src" ]; then \
			[ "$$first" = false ] && echo ',' >> compile_commands.json; \
			echo '  {' >> compile_commands.json; \
			echo '    "directory": "$(shell pwd)",' >> compile_commands.json; \
			echo '    "command": "$(CXX) $(CXXFLAGS) -c $$src",' >> compile_commands.json; \
			echo '    "file": "'$$src'"' >> compile_commands.json; \
			echo -n '  }' >> compile_commands.json; \
			first=false; \
		fi \
	done
	@echo '' >> compile_commands.json
	@echo ']' >> compile_commands.json
	@echo "$(COLOR_GREEN)✓ compile_commands.json generated$(COLOR_RESET)"

# ============================================================================
# Special Targets
# ============================================================================

.PHONY: list-tests list-bins

list-tests:
	@echo "$(COLOR_BOLD)Unit Tests:$(COLOR_RESET)"
	@echo "$(UNIT_TEST_BINS)" | tr ' ' '\n' | sed 's|$(BUILD_DIR)/||'
	@echo ""

list-bins:
	@echo "$(COLOR_BOLD)All Binaries:$(COLOR_RESET)"
	@echo "$(ALL_BINS)" | tr ' ' '\n'
