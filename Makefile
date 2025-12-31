CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O0 -pg
TARGET = build/lisp_vm
TEST_TAG = build/test_tagging
SIMPLE_TAG = build/simple_tagging_test
TEST_VARS = build/test_variables
TEST_LAMBDA = build/test_lambda
TEST_LOOPS = build/test_loops
TEST_MICRO = build/test_microcode
TEST_ADVANCED = build/test_advanced
TEST_SMALLTALK = build/test_smalltalk
TEST_COMMENTS = build/test_comments
TEST_VM_STACK = build/test_vm_stack
TEST_VM_ALU = build/test_vm_alu
TEST_VM_MEMORY = build/test_vm_memory
TEST_VM_CONTROL = build/test_vm_control
TEST_PARSER_BASIC = build/test_parser_basic
TEST_PARSER_COMMENTS = build/test_parser_comments
TEST_PARSER_ERRORS = build/test_parser_errors
TEST_COMPILER_BASIC = build/test_compiler_basic
TEST_COMPILER_CONTROL = build/test_compiler_control
TEST_COMPILER_VARIABLES = build/test_compiler_variables
TEST_COMPILER_FUNCTIONS = build/test_compiler_functions
TEST_TRANSPILER = build/test_transpiler
TEST_TRANSPILER_EXT = build/test_transpiler_extended
TRANSPILER_DEMO = build/transpiler_demo
TOKENIZER_TRANSPILER = build/transpile_tokenizer
ST_TOKENIZER = build/st_tokenizer_file

all: $(TARGET) $(TEST_TAG) $(SIMPLE_TAG) $(TEST_VARS) $(TEST_LAMBDA) $(TEST_LOOPS) $(TEST_MICRO) $(TEST_ADVANCED) $(TEST_SMALLTALK) $(TEST_COMMENTS) $(TEST_VM_STACK) $(TEST_VM_ALU) $(TEST_VM_MEMORY) $(TEST_VM_CONTROL) $(TEST_PARSER_BASIC) $(TEST_PARSER_COMMENTS) $(TEST_PARSER_ERRORS) $(TEST_COMPILER_BASIC) $(TEST_COMPILER_CONTROL) $(TEST_COMPILER_VARIABLES) $(TEST_COMPILER_FUNCTIONS) $(TEST_TRANSPILER) $(TEST_TRANSPILER_EXT) $(TRANSPILER_DEMO)

$(TARGET): src/main.cpp src/stack_vm.hpp src/lisp_parser.hpp src/lisp_compiler.hpp
	$(CXX) $(CXXFLAGS) -o $(TARGET) src/main.cpp

$(TEST_TAG): src/test_tagging.cpp src/stack_vm.hpp src/lisp_parser.hpp src/lisp_compiler.hpp
	$(CXX) $(CXXFLAGS) -o $(TEST_TAG) src/test_tagging.cpp

$(SIMPLE_TAG): src/simple_tagging_test.cpp src/stack_vm.hpp src/lisp_parser.hpp src/lisp_compiler.hpp
	$(CXX) $(CXXFLAGS) -o $(SIMPLE_TAG) src/simple_tagging_test.cpp

$(TEST_VARS): src/test_variables.cpp src/stack_vm.hpp src/lisp_parser.hpp src/lisp_compiler.hpp
	$(CXX) $(CXXFLAGS) -o $(TEST_VARS) src/test_variables.cpp

$(TEST_LAMBDA): src/test_lambda.cpp src/stack_vm.hpp src/lisp_parser.hpp src/lisp_compiler.hpp
	$(CXX) $(CXXFLAGS) -o $(TEST_LAMBDA) src/test_lambda.cpp

$(TEST_LOOPS): src/test_loops.cpp src/stack_vm.hpp src/lisp_parser.hpp src/lisp_compiler.hpp
	$(CXX) $(CXXFLAGS) -o $(TEST_LOOPS) src/test_loops.cpp

$(TEST_MICRO): src/test_microcode.cpp src/stack_vm.hpp src/lisp_parser.hpp src/lisp_compiler.hpp src/microcode.hpp
	$(CXX) $(CXXFLAGS) -o $(TEST_MICRO) src/test_microcode.cpp

$(TEST_ADVANCED): src/test_advanced.cpp src/stack_vm.hpp src/lisp_parser.hpp src/lisp_compiler.hpp src/microcode.hpp
	$(CXX) $(CXXFLAGS) -o $(TEST_ADVANCED) src/test_advanced.cpp

$(TEST_SMALLTALK): src/test_smalltalk.cpp src/stack_vm.hpp src/lisp_parser.hpp src/lisp_compiler.hpp src/microcode.hpp
	$(CXX) $(CXXFLAGS) -o $(TEST_SMALLTALK) src/test_smalltalk.cpp

$(TEST_COMMENTS): src/test_comments.cpp src/stack_vm.hpp src/lisp_parser.hpp src/lisp_compiler.hpp
	$(CXX) $(CXXFLAGS) -o $(TEST_COMMENTS) src/test_comments.cpp

$(TEST_VM_STACK): tests/test_vm_stack.cpp src/stack_vm.hpp
	$(CXX) $(CXXFLAGS) -o $(TEST_VM_STACK) tests/test_vm_stack.cpp

$(TEST_VM_ALU): tests/test_vm_alu.cpp src/stack_vm.hpp
	$(CXX) $(CXXFLAGS) -o $(TEST_VM_ALU) tests/test_vm_alu.cpp

$(TEST_VM_MEMORY): tests/test_vm_memory.cpp src/stack_vm.hpp
	$(CXX) $(CXXFLAGS) -o $(TEST_VM_MEMORY) tests/test_vm_memory.cpp

$(TEST_VM_CONTROL): tests/test_vm_control.cpp src/stack_vm.hpp
	$(CXX) $(CXXFLAGS) -o $(TEST_VM_CONTROL) tests/test_vm_control.cpp

$(TEST_PARSER_BASIC): tests/test_parser_basic.cpp src/lisp_parser.hpp
	$(CXX) $(CXXFLAGS) -o $(TEST_PARSER_BASIC) tests/test_parser_basic.cpp

$(TEST_PARSER_COMMENTS): tests/test_parser_comments.cpp src/lisp_parser.hpp
	$(CXX) $(CXXFLAGS) -o $(TEST_PARSER_COMMENTS) tests/test_parser_comments.cpp

$(TEST_PARSER_ERRORS): tests/test_parser_errors.cpp src/lisp_parser.hpp
	$(CXX) $(CXXFLAGS) -o $(TEST_PARSER_ERRORS) tests/test_parser_errors.cpp

$(TEST_COMPILER_BASIC): tests/test_compiler_basic.cpp src/stack_vm.hpp src/lisp_parser.hpp src/lisp_compiler.hpp
	$(CXX) $(CXXFLAGS) -o $(TEST_COMPILER_BASIC) tests/test_compiler_basic.cpp

$(TEST_COMPILER_CONTROL): tests/test_compiler_control.cpp src/stack_vm.hpp src/lisp_parser.hpp src/lisp_compiler.hpp
	$(CXX) $(CXXFLAGS) -o $(TEST_COMPILER_CONTROL) tests/test_compiler_control.cpp

$(TEST_COMPILER_VARIABLES): tests/test_compiler_variables.cpp src/stack_vm.hpp src/lisp_parser.hpp src/lisp_compiler.hpp
	$(CXX) $(CXXFLAGS) -o $(TEST_COMPILER_VARIABLES) tests/test_compiler_variables.cpp

$(TEST_COMPILER_FUNCTIONS): tests/test_compiler_functions.cpp src/stack_vm.hpp src/lisp_parser.hpp src/lisp_compiler.hpp
	$(CXX) $(CXXFLAGS) -o $(TEST_COMPILER_FUNCTIONS) tests/test_compiler_functions.cpp

$(TEST_TRANSPILER): tests/test_transpiler.cpp src/lisp_parser.hpp src/lisp_to_cpp.hpp
	$(CXX) $(CXXFLAGS) -o $(TEST_TRANSPILER) tests/test_transpiler.cpp

$(TEST_TRANSPILER_EXT): tests/test_transpiler_extended.cpp src/lisp_parser.hpp src/lisp_to_cpp.hpp
	$(CXX) $(CXXFLAGS) -o $(TEST_TRANSPILER_EXT) tests/test_transpiler_extended.cpp

$(TRANSPILER_DEMO): examples/transpiler_demo.cpp src/lisp_parser.hpp src/lisp_to_cpp.hpp
	$(CXX) $(CXXFLAGS) -o $(TRANSPILER_DEMO) examples/transpiler_demo.cpp

$(TOKENIZER_TRANSPILER): src/transpile_tokenizer.cpp src/lisp_parser.hpp src/lisp_to_cpp.hpp
	$(CXX) $(CXXFLAGS) -o $(TOKENIZER_TRANSPILER) src/transpile_tokenizer.cpp

$(ST_TOKENIZER): src/tokenizer_main.cpp src/lisp_parser.hpp
	$(CXX) $(CXXFLAGS) -o $(ST_TOKENIZER) src/tokenizer_main.cpp

clean:
	rm -f $(TARGET) $(TEST_TAG) $(SIMPLE_TAG) $(TEST_VARS) $(TEST_LAMBDA) $(TEST_LOOPS) $(TEST_MICRO) $(TEST_ADVANCED) $(TEST_SMALLTALK) $(TEST_COMMENTS) $(TEST_VM_STACK) $(TEST_VM_ALU) $(TEST_VM_MEMORY) $(TEST_VM_CONTROL) $(TEST_PARSER_BASIC) $(TEST_PARSER_COMMENTS) $(TEST_PARSER_ERRORS) $(TEST_COMPILER_BASIC) $(TEST_COMPILER_CONTROL) $(TEST_COMPILER_VARIABLES) $(TEST_COMPILER_FUNCTIONS) $(TEST_TRANSPILER) $(TEST_TRANSPILER_EXT) $(TRANSPILER_DEMO)
	rm -f build/test_*.cpp build/test_add build/test_sub build/test_mul build/test_div build/test_mod build/test_multi_* build/test_eq_* build/test_lt_* build/test_gt_* build/test_nested* build/test_define_* build/test_multiple_* build/test_set_* build/test_if_* build/test_while_* build/test_for_* build/test_simple_* build/test_two_* build/test_function_* build/test_ffi_* build/test_bitwise_*
	rm -f build/test_ext_* build/example_*.cpp build/example_loop

run: $(TARGET)
	./$(TARGET)

test: $(TEST_TAG)
	./$(TEST_TAG)

simple: $(SIMPLE_TAG)
	./$(SIMPLE_TAG)

vars: $(TEST_VARS)
	./$(TEST_VARS)

lambda: $(TEST_LAMBDA)
	./$(TEST_LAMBDA)

loops: $(TEST_LOOPS)
	./$(TEST_LOOPS)

micro: $(TEST_MICRO)
	./$(TEST_MICRO)

advanced: $(TEST_ADVANCED)
	./$(TEST_ADVANCED)

smalltalk: $(TEST_SMALLTALK)
	./$(TEST_SMALLTALK)

comments: $(TEST_COMMENTS)
	./$(TEST_COMMENTS)

vm-stack: $(TEST_VM_STACK)
	./$(TEST_VM_STACK)

vm-alu: $(TEST_VM_ALU)
	./$(TEST_VM_ALU)

vm-memory: $(TEST_VM_MEMORY)
	./$(TEST_VM_MEMORY)

vm-control: $(TEST_VM_CONTROL)
	./$(TEST_VM_CONTROL)

vm-all: vm-stack vm-alu vm-memory vm-control
	@echo ""
	@echo "✓ All VM tests passed!"

parser-basic: $(TEST_PARSER_BASIC)
	./$(TEST_PARSER_BASIC)

parser-comments: $(TEST_PARSER_COMMENTS)
	./$(TEST_PARSER_COMMENTS)

parser-errors: $(TEST_PARSER_ERRORS)
	./$(TEST_PARSER_ERRORS)

parser-all: parser-basic parser-comments parser-errors
	@echo ""
	@echo "✓ All parser tests passed!"

compiler-basic: $(TEST_COMPILER_BASIC)
	./$(TEST_COMPILER_BASIC)

compiler-control: $(TEST_COMPILER_CONTROL)
	./$(TEST_COMPILER_CONTROL)

compiler-variables: $(TEST_COMPILER_VARIABLES)
	./$(TEST_COMPILER_VARIABLES)

compiler-functions: $(TEST_COMPILER_FUNCTIONS)
	./$(TEST_COMPILER_FUNCTIONS)

compiler-all: compiler-basic compiler-control compiler-variables compiler-functions
	@echo ""
	@echo "✓ All compiler tests passed!"

transpiler: $(TEST_TRANSPILER) $(TEST_TRANSPILER_EXT)
	./$(TEST_TRANSPILER)
	@echo ""
	./$(TEST_TRANSPILER_EXT)

transpiler-demo: $(TRANSPILER_DEMO)
	./$(TRANSPILER_DEMO)

tokenizer: $(TOKENIZER_TRANSPILER)
	@echo "=== Transpiling Smalltalk Tokenizer ==="
	./$(TOKENIZER_TRANSPILER)
	@echo ""
	@echo "=== Compiling Generated C++ ==="
	g++ -std=c++17 -o build/st_tokenizer build/smalltalk_tokenizer.cpp
	@echo ""
	@echo "=== Running Tokenizer ==="
	./build/st_tokenizer

tokenizer-transpile: $(TOKENIZER_TRANSPILER)
	@echo "=== Regenerating tokenizer_main.cpp from Lisp source ==="
	./$(TOKENIZER_TRANSPILER) lisp/smalltalk_tokenizer_ffi.lisp build/tokenizer_main_tmp.cpp
	@echo ""
	@echo "Adding header comments..."
	@echo "// AUTO-GENERATED CODE - DO NOT EDIT MANUALLY" > src/tokenizer_main.cpp
	@echo "// This file was transpiled from: lisp/smalltalk_tokenizer_ffi.lisp" >> src/tokenizer_main.cpp
	@echo "// To regenerate, run: make tokenizer-transpile" >> src/tokenizer_main.cpp
	@echo "//" >> src/tokenizer_main.cpp
	@echo "// Smalltalk Tokenizer with FFI for file I/O" >> src/tokenizer_main.cpp
	@echo "// Demonstrates the Lisp-to-C++ transpiler with FFI capabilities" >> src/tokenizer_main.cpp
	@echo "" >> src/tokenizer_main.cpp
	@tail -n +2 build/tokenizer_main_tmp.cpp >> src/tokenizer_main.cpp
	@rm build/tokenizer_main_tmp.cpp
	@echo "✓ tokenizer_main.cpp regenerated successfully!"

tokenizer-file: $(ST_TOKENIZER)
	@echo "=== Smalltalk Tokenizer with File I/O ==="
	@echo ""
	@if [ -f examples/test.st ]; then \
		echo "Testing on examples/test.st:"; \
		./$(ST_TOKENIZER) examples/test.st; \
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
	@echo "=== Transpiling Smalltalk Parser ==="
	./$(TOKENIZER_TRANSPILER) lisp/smalltalk_parser.lisp build/smalltalk_parser.cpp
	@echo ""
	@echo "=== Compiling Generated C++ ==="
	g++ -std=c++17 -o build/st_parser build/smalltalk_parser.cpp
	@echo ""
	@echo "=== Running Parser ==="
	./build/st_parser

typed-functions: $(TOKENIZER_TRANSPILER)
	@echo "=== Transpiling Typed Functions Test ==="
	./$(TOKENIZER_TRANSPILER) lisp/test_typed_functions.lisp build/test_typed_functions.cpp
	@echo ""
	@echo "=== Compiling Generated C++ ==="
	g++ -std=c++17 -o build/test_typed_functions build/test_typed_functions.cpp
	@echo ""
	@echo "=== Running Typed Functions Test ==="
	./build/test_typed_functions

parser-typed: $(TOKENIZER_TRANSPILER)
	@echo "=== Transpiling Smalltalk Parser (Typed Strings) ==="
	./$(TOKENIZER_TRANSPILER) lisp/smalltalk_parser_typed.lisp build/smalltalk_parser_typed.cpp
	@echo ""
	@echo "=== Compiling Generated C++ ==="
	g++ -std=c++17 -o build/st_parser_typed build/smalltalk_parser_typed.cpp
	@echo ""
	@echo "=== Running Smalltalk Parser ==="
	./build/st_parser_typed

integration-all: test simple vars lambda loops micro advanced smalltalk comments
	@echo ""
	@echo "✓ All integration tests passed!"

test-all: vm-all parser-all compiler-all transpiler integration-all
	@echo ""
	@echo "======================================"
	@echo "✓ ALL TESTS PASSED!"
	@echo "======================================"
	@echo "  Unit Tests:"
	@echo "    ✓ VM tests (43 tests)"
	@echo "    ✓ Parser tests (58 tests)"
	@echo "    ✓ Compiler tests (44 tests)"
	@echo "    ✓ Transpiler tests (35 tests)"
	@echo "  Integration Tests:"
	@echo "    ✓ Tagging, Variables, Lambda"
	@echo "    ✓ Loops, Microcode, Advanced"
	@echo "    ✓ Smalltalk, Comments"
	@echo "======================================"

.PHONY: all clean run test simple vars lambda loops micro advanced smalltalk comments vm-stack vm-alu vm-memory vm-control vm-all parser-basic parser-comments parser-errors parser-all compiler-basic compiler-control compiler-variables compiler-functions compiler-all transpiler transpiler-demo tokenizer tokenizer-transpile tokenizer-file integration-all test-all
