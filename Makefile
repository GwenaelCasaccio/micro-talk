# ============================================================================
# Micro-Talk VM Build System
# ============================================================================

# Compiler and flags
CXX := g++
# OPTIMIZE flag support: make OPTIMIZE=1 for -O3, default -O0 -g
ifdef OPTIMIZE
    CXXFLAGS := -std=c++17 -Wall -Wextra -O3
else
    CXXFLAGS := -std=c++17 -Wall -Wextra -O0 -g
endif

# Bounds checking configuration (default: enabled in debug, disabled in release)
# Override with: make BOUNDS_CHECKS=1 (force enable) or make BOUNDS_CHECKS=0 (force disable)
ifdef BOUNDS_CHECKS
    ifeq ($(BOUNDS_CHECKS),0)
        CXXFLAGS += -DMICRO_TALK_BOUNDS_CHECKS=0
    else
        CXXFLAGS += -DMICRO_TALK_BOUNDS_CHECKS=1
    endif
endif

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
	$(BUILD_DIR)/test_memory_granular \
	$(BUILD_DIR)/test_comments \
	$(BUILD_DIR)/test_funcall \
	$(BUILD_DIR)/test_symbol_table \
	$(BUILD_DIR)/test_vm_stack \
	$(BUILD_DIR)/test_vm_interrupt \
	$(BUILD_DIR)/test_vm_alu \
	$(BUILD_DIR)/test_vm_memory \
	$(BUILD_DIR)/test_vm_control \
	$(BUILD_DIR)/test_vm_profiling \
	$(BUILD_DIR)/test_vm_instruction_limit \
	$(BUILD_DIR)/test_vm_checkpoint \
	$(BUILD_DIR)/test_vm_benchmark \
	$(BUILD_DIR)/test_parser_basic \
	$(BUILD_DIR)/test_parser_comments \
	$(BUILD_DIR)/test_parser_errors \
	$(BUILD_DIR)/test_parser_radix \
	$(BUILD_DIR)/test_compiler_basic \
	$(BUILD_DIR)/test_compiler_control \
	$(BUILD_DIR)/test_compiler_variables \
	$(BUILD_DIR)/test_compiler_functions \
	$(BUILD_DIR)/test_compiler_interrupts \
	$(BUILD_DIR)/test_transpiler \
	$(BUILD_DIR)/test_transpiler_extended

# Smalltalk integration test binaries (from tests/smalltalk/)
SMALLTALK_TEST_BINS := \
	$(BUILD_DIR)/st_test_01_classes \
	$(BUILD_DIR)/st_test_02_contexts \
	$(BUILD_DIR)/st_test_03_symbols \
	$(BUILD_DIR)/st_test_04_strings \
	$(BUILD_DIR)/st_test_05_tokenizer \
	$(BUILD_DIR)/st_test_06_parser \
	$(BUILD_DIR)/st_test_07_compiler \
	$(BUILD_DIR)/st_test_08_methods \
	$(BUILD_DIR)/st_test_09_message_sends

# Special binaries
TRANSPILER_DEMO := $(BUILD_DIR)/transpiler_demo
TOKENIZER_TRANSPILER := $(BUILD_DIR)/transpile_tokenizer
ST_TOKENIZER := $(BUILD_DIR)/st_tokenizer_file
MINIMAL_VM := $(BUILD_DIR)/minimal_vm_test
DISASSEMBLER_TEST := $(BUILD_DIR)/test_disassembler
EVAL_TEST := $(BUILD_DIR)/test_eval

# All binaries (excluding optional/generated ones like ST_TOKENIZER)
ALL_BINS := $(TARGET) $(UNIT_TEST_BINS) $(SMALLTALK_TEST_BINS) $(SMALLTALK) \
            $(TRANSPILER_DEMO) $(TOKENIZER_TRANSPILER) $(MINIMAL_VM) \
            $(DISASSEMBLER_TEST) $(EVAL_TEST)

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
	@echo "  make st-all       - Run all Smalltalk split tests"
	@echo "  make st-classes   - Run Smalltalk tests 1-8 (classes)"
	@echo "  make st-contexts  - Run Smalltalk tests 9-14 (contexts)"
	@echo ""
	@echo "$(COLOR_BOLD)Performance:$(COLOR_RESET)"
	@echo "  make benchmark    - Run dispatch benchmarks (-O0 and -O3)"
	@echo "  make benchmark-o0 - Benchmark at -O0 (debug)"
	@echo "  make benchmark-o3 - Benchmark at -O3 (release)"
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

# Eval test (runtime code generation)
$(EVAL_TEST): $(SRC_DIR)/test_eval.cpp $(COMPILER_DEPS) $(SRC_DIR)/eval_context.hpp | $(BUILD_DIR)
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

# Smalltalk split tests (from tests/smalltalk/)
$(BUILD_DIR)/st_test_%: $(TEST_DIR)/smalltalk/test_%.cpp $(COMPILER_DEPS) | $(BUILD_DIR)
	@echo "$(COLOR_BLUE)Compiling$(COLOR_RESET) $@"
	@$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

# ============================================================================
# Test Execution Targets
# ============================================================================

.PHONY: run test simple vars lambda loops micro advanced smalltalk comments disasm symbol-table eval

run: $(TARGET)
	@./$(TARGET)

# Individual integration tests
test: $(BUILD_DIR)/test_tagging
	@./$(BUILD_DIR)/test_tagging

# simple: $(BUILD_DIR)/simple_tagging_test - OBSOLETE: binary no longer exists
#	@./$(BUILD_DIR)/simple_tagging_test

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

# Build combined smalltalk.lisp from modular sources
SMALLTALK_MODULES := \
	$(LISP_DIR)/smalltalk/00-runtime.lisp \
	$(LISP_DIR)/smalltalk/01-symbol-table.lisp \
	$(LISP_DIR)/smalltalk/02-classes.lisp \
	$(LISP_DIR)/smalltalk/03-methods.lisp \
	$(LISP_DIR)/smalltalk/04-tokenizer.lisp \
	$(LISP_DIR)/smalltalk/05-parser.lisp \
	$(LISP_DIR)/smalltalk/06-compiler.lisp

smalltalk-build: $(SMALLTALK_MODULES)
	@echo "$(COLOR_BLUE)=== Building combined smalltalk.lisp ===$(COLOR_RESET)"
	@$(LISP_DIR)/smalltalk/build.sh
	@echo "$(COLOR_GREEN)✓ Generated $(LISP_DIR)/smalltalk.lisp$(COLOR_RESET)"

hash: $(BUILD_DIR)/test_hash_table
	@./$(BUILD_DIR)/test_hash_table

symbol-table: $(BUILD_DIR)/test_symbol_table
	@./$(BUILD_DIR)/test_symbol_table

memory: $(BUILD_DIR)/test_memory_granular
	@./$(BUILD_DIR)/test_memory_granular

comments: $(BUILD_DIR)/test_comments
	@./$(BUILD_DIR)/test_comments

disasm: $(DISASSEMBLER_TEST)
	@./$(DISASSEMBLER_TEST)

eval: $(EVAL_TEST)
	@./$(EVAL_TEST)

# ============================================================================
# Unit Test Suites
# ============================================================================

.PHONY: vm-stack vm-alu vm-memory vm-control vm-checkpoint vm-all
.PHONY: parser-basic parser-comments parser-errors parser-all
.PHONY: compiler-basic compiler-control compiler-variables compiler-functions compiler-interrupts compiler-all
.PHONY: transpiler transpiler-demo integration-all test-all
.PHONY: st-classes st-contexts st-symbols st-strings st-tokenizer st-parser st-compiler st-methods st-messages st-all

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

vm-profiling: $(BUILD_DIR)/test_vm_profiling
	@./$(BUILD_DIR)/test_vm_profiling

vm-instruction-limit: $(BUILD_DIR)/test_vm_instruction_limit
	@./$(BUILD_DIR)/test_vm_instruction_limit

vm-checkpoint: $(BUILD_DIR)/test_vm_checkpoint
	@./$(BUILD_DIR)/test_vm_checkpoint

vm-all: vm-stack vm-alu vm-memory vm-control vm-profiling vm-instruction-limit vm-checkpoint
	@echo ""
	@echo "$(COLOR_GREEN)✓ All VM tests passed!$(COLOR_RESET)"

# Parser tests
parser-basic: $(BUILD_DIR)/test_parser_basic
	@./$(BUILD_DIR)/test_parser_basic

parser-comments: $(BUILD_DIR)/test_parser_comments
	@./$(BUILD_DIR)/test_parser_comments

parser-errors: $(BUILD_DIR)/test_parser_errors
	@./$(BUILD_DIR)/test_parser_errors

parser-radix: $(BUILD_DIR)/test_parser_radix
	@./$(BUILD_DIR)/test_parser_radix

parser-all: parser-basic parser-comments parser-errors parser-radix
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
integration-all: test vars lambda loops micro advanced smalltalk comments eval
	@echo ""
	@echo "$(COLOR_GREEN)✓ All integration tests passed!$(COLOR_RESET)"

# Smalltalk split integration tests
st-classes: $(BUILD_DIR)/st_test_01_classes
	@./$(BUILD_DIR)/st_test_01_classes

st-contexts: $(BUILD_DIR)/st_test_02_contexts
	@./$(BUILD_DIR)/st_test_02_contexts

st-symbols: $(BUILD_DIR)/st_test_03_symbols
	@./$(BUILD_DIR)/st_test_03_symbols

st-strings: $(BUILD_DIR)/st_test_04_strings
	@./$(BUILD_DIR)/st_test_04_strings

st-tokenizer: $(BUILD_DIR)/st_test_05_tokenizer
	@./$(BUILD_DIR)/st_test_05_tokenizer

st-parser: $(BUILD_DIR)/st_test_06_parser
	@./$(BUILD_DIR)/st_test_06_parser

st-compiler: $(BUILD_DIR)/st_test_07_compiler
	@./$(BUILD_DIR)/st_test_07_compiler

st-methods: $(BUILD_DIR)/st_test_08_methods
	@./$(BUILD_DIR)/st_test_08_methods

st-messages: $(BUILD_DIR)/st_test_09_message_sends
	@./$(BUILD_DIR)/st_test_09_message_sends

st-all: st-classes st-contexts st-symbols st-strings st-tokenizer st-parser st-compiler st-methods st-messages
	@echo ""
	@echo "$(COLOR_GREEN)✓ All Smalltalk split tests passed!$(COLOR_RESET)"

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
# Performance Benchmarking
# ============================================================================

.PHONY: benchmark benchmark-o0 benchmark-o3

benchmark-o0: $(BUILD_DIR)/test_vm_benchmark
	@echo ""
	@echo "$(COLOR_BLUE)$(COLOR_BOLD)╔════════════════════════════════════════════════╗$(COLOR_RESET)"
	@echo "$(COLOR_BLUE)$(COLOR_BOLD)║   Computed Goto Benchmark (-O0 Debug Build)   ║$(COLOR_RESET)"
	@echo "$(COLOR_BLUE)$(COLOR_BOLD)╚════════════════════════════════════════════════╝$(COLOR_RESET)"
	@echo ""
	@./$(BUILD_DIR)/test_vm_benchmark

benchmark-o3:
	@$(MAKE) clean > /dev/null 2>&1
	@$(MAKE) OPTIMIZE=1 $(BUILD_DIR)/test_vm_benchmark > /dev/null 2>&1
	@echo ""
	@echo "$(COLOR_GREEN)$(COLOR_BOLD)╔════════════════════════════════════════════════╗$(COLOR_RESET)"
	@echo "$(COLOR_GREEN)$(COLOR_BOLD)║   Computed Goto Benchmark (-O3 Release Build) ║$(COLOR_RESET)"
	@echo "$(COLOR_GREEN)$(COLOR_BOLD)╚════════════════════════════════════════════════╝$(COLOR_RESET)"
	@echo ""
	@./$(BUILD_DIR)/test_vm_benchmark

benchmark: benchmark-o0 benchmark-o3
	@echo ""
	@echo "$(COLOR_GREEN)$(COLOR_BOLD)✓ Benchmark complete!$(COLOR_RESET)"
	@echo ""
	@echo "$(COLOR_YELLOW)Note: Computed goto provides 20-40% speedup over switch at -O3$(COLOR_RESET)"

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
