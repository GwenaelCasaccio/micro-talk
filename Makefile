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

all: $(TARGET) $(TEST_TAG) $(SIMPLE_TAG) $(TEST_VARS) $(TEST_LAMBDA) $(TEST_LOOPS) $(TEST_MICRO) $(TEST_ADVANCED) $(TEST_SMALLTALK) $(TEST_COMMENTS) $(TEST_VM_STACK) $(TEST_VM_ALU) $(TEST_VM_MEMORY) $(TEST_VM_CONTROL)

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

clean:
	rm -f $(TARGET) $(TEST_TAG) $(SIMPLE_TAG) $(TEST_VARS) $(TEST_LAMBDA) $(TEST_LOOPS) $(TEST_MICRO) $(TEST_ADVANCED) $(TEST_SMALLTALK) $(TEST_COMMENTS) $(TEST_VM_STACK) $(TEST_VM_ALU) $(TEST_VM_MEMORY) $(TEST_VM_CONTROL)

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

.PHONY: all clean run test simple vars lambda loops micro advanced smalltalk comments vm-stack vm-alu vm-memory vm-control vm-all
