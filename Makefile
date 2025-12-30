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

all: $(TARGET) $(TEST_TAG) $(SIMPLE_TAG) $(TEST_VARS) $(TEST_LAMBDA) $(TEST_LOOPS) $(TEST_MICRO) $(TEST_ADVANCED) $(TEST_SMALLTALK)

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

clean:
	rm -f $(TARGET) $(TEST_TAG) $(SIMPLE_TAG) $(TEST_VARS) $(TEST_LAMBDA) $(TEST_LOOPS) $(TEST_MICRO) $(TEST_ADVANCED) $(TEST_SMALLTALK)

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

.PHONY: all clean run test simple vars lambda loops micro advanced smalltalk
