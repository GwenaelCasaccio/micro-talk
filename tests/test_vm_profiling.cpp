#include "../src/lisp_compiler.hpp"
#include "../src/lisp_parser.hpp"
#include "../src/stack_vm.hpp"
#include <iostream>

// Test profiling functionality
void test_basic_profiling() {
    std::cout << "=== Test 1: Basic Profiling ===" << std::endl;

    StackVM vm;
    LispCompiler compiler;

    // Simple arithmetic program
    const char* code = R"(
        (do
            (+ 10 20)
            (* 5 6)
            (- 100 50)
            (/ 84 2))
    )";

    LispParser parser(code);
    auto ast = parser.parse();
    auto program = compiler.compile(ast);
    vm.load_program(program);

    // Enable profiling
    vm.enable_profiling();

    // Execute
    vm.execute();

    // Print profiling report
    vm.print_profiling_report();

    std::cout << "\n✓ Test 1 passed\n" << std::endl;
}

void test_profiling_with_loops() {
    std::cout << "=== Test 2: Profiling with Loops ===" << std::endl;

    StackVM vm;
    LispCompiler compiler;

    // Program with loop - calculate sum of 1 to 10
    const char* code = R"(
        (do
            (for (i 1 11)
                (* i 2))
            (+ 100 200))
    )";

    LispParser parser(code);
    auto ast = parser.parse();
    auto program = compiler.compile(ast);
    vm.load_program(program);

    // Enable profiling and reset counters
    vm.enable_profiling();
    vm.reset_profiling();

    // Execute
    vm.execute();

    std::cout << "\nResult: " << vm.get_top() << " (expected: 300)" << std::endl;

    // Print profiling report
    vm.print_profiling_report();

    // Verify some expected opcodes were executed
    assert(vm.get_opcode_count(Opcode::PUSH) > 0);
    assert(vm.get_opcode_count(Opcode::ADD) > 0);
    assert(vm.get_total_instructions() > 0);

    std::cout << "\n✓ Test 2 passed\n" << std::endl;
}

void test_profiling_enable_disable() {
    std::cout << "=== Test 3: Enable/Disable Profiling ===" << std::endl;

    StackVM vm;
    LispCompiler compiler;

    const char* code = "(+ 1 2)";
    LispParser parser(code);
    auto ast = parser.parse();
    auto program = compiler.compile(ast);

    // Run without profiling
    vm.load_program(program);
    vm.execute();
    assert(vm.get_total_instructions() == 0);
    std::cout << "  ✓ Instructions not counted when profiling disabled" << std::endl;

    // Run with profiling
    vm.reset();
    vm.enable_profiling();
    vm.reset_profiling();
    vm.execute();
    assert(vm.get_total_instructions() > 0);
    std::cout << "  ✓ Instructions counted when profiling enabled (" << vm.get_total_instructions()
              << " instructions)" << std::endl;

    // Disable and verify
    vm.disable_profiling();
    assert(!vm.is_profiling_enabled());
    std::cout << "  ✓ Profiling can be disabled" << std::endl;

    std::cout << "\n✓ Test 3 passed\n" << std::endl;
}

void test_profiling_snapshot() {
    std::cout << "=== Test 4: Profiling with Checkpoint/Restore ===" << std::endl;

    StackVM vm;
    LispCompiler compiler;

    const char* code = R"(
        (do
            (* 6 7)
            (+ 10 20))
    )";

    LispParser parser(code);
    auto ast = parser.parse();
    auto program = compiler.compile(ast);
    vm.load_program(program);
    vm.enable_profiling();
    vm.reset_profiling();

    // Execute first part
    vm.execute();

    // Save profiling stats
    uint64_t instructions_before = vm.get_total_instructions();
    auto snapshot = vm.checkpoint();

    std::cout << "  Total instructions before checkpoint: " << instructions_before << std::endl;

    // Reset and execute again (this would add more instructions)
    vm.reset();
    vm.reset_profiling();
    vm.execute();

    uint64_t instructions_after_reset = vm.get_total_instructions();
    std::cout << "  Total instructions after reset: " << instructions_after_reset << std::endl;

    // Restore snapshot
    vm.restore(snapshot);

    // Verify profiling data was restored
    assert(vm.get_total_instructions() == instructions_before);
    assert(vm.is_profiling_enabled());
    std::cout << "  ✓ Profiling data restored correctly: " << vm.get_total_instructions()
              << " instructions" << std::endl;

    std::cout << "\n✓ Test 4 passed\n" << std::endl;
}

int main() {
    std::cout << "=== VM Profiling Tests ===" << std::endl;

    test_basic_profiling();
    test_profiling_with_loops();
    test_profiling_enable_disable();
    test_profiling_snapshot();

    std::cout << "\n=== All Profiling Tests Passed ===" << std::endl;

    return 0;
}
