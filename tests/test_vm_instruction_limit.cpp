#include "../src/stack_vm.hpp"
#include <iostream>

void test_instruction_limit_infinite_loop() {
    std::cout << "=== Test 1: Infinite Loop with Instruction Limit ===" << std::endl;

    StackVM vm;

    // Create an infinite loop: JMP to itself
    // Address 0: JMP 0
    std::vector<uint64_t> program = {
        static_cast<uint64_t>(Opcode::JMP),
        0 // Jump to address 0 (infinite loop)
    };

    vm.load_program(program);

    // Execute with limit of 1000 instructions
    vm.execute(1000);

    // Verify that we hit the instruction limit
    assert(vm.hit_instruction_limit_check());
    std::cout << "  ✓ Execution stopped after hitting instruction limit" << std::endl;

    std::cout << "\n✓ Test 1 passed\n" << std::endl;
}

void test_instruction_limit_normal_program() {
    std::cout << "=== Test 2: Normal Program with High Limit ===" << std::endl;

    StackVM vm;

    // Simple program that halts normally
    // PUSH 10, PUSH 20, ADD, HALT
    std::vector<uint64_t> program = {
        static_cast<uint64_t>(Opcode::PUSH), 10,
        static_cast<uint64_t>(Opcode::PUSH), 20,
        static_cast<uint64_t>(Opcode::ADD),  static_cast<uint64_t>(Opcode::HALT)};

    vm.load_program(program);

    // Execute with high limit
    vm.execute(10000);

    // Verify that we did NOT hit the instruction limit
    assert(!vm.hit_instruction_limit_check());
    assert(vm.get_top() == 30);
    std::cout << "  ✓ Program completed normally without hitting limit" << std::endl;
    std::cout << "  ✓ Result: " << vm.get_top() << " (expected: 30)" << std::endl;

    std::cout << "\n✓ Test 2 passed\n" << std::endl;
}

void test_instruction_limit_no_limit() {
    std::cout << "=== Test 3: Program with No Limit (Default) ===" << std::endl;

    StackVM vm;

    // Simple program
    std::vector<uint64_t> program = {static_cast<uint64_t>(Opcode::PUSH), 42,
                                     static_cast<uint64_t>(Opcode::HALT)};

    vm.load_program(program);

    // Execute with default (no limit)
    vm.execute();

    // Verify normal completion
    assert(!vm.hit_instruction_limit_check());
    assert(vm.get_top() == 42);
    std::cout << "  ✓ Program executed with unlimited instructions" << std::endl;

    std::cout << "\n✓ Test 3 passed\n" << std::endl;
}

void test_instruction_limit_exact_boundary() {
    std::cout << "=== Test 4: Exact Boundary Test ===" << std::endl;

    StackVM vm;

    // Program with exactly 4 instructions
    // PUSH 1, PUSH 2, ADD, HALT
    std::vector<uint64_t> program = {
        static_cast<uint64_t>(Opcode::PUSH), 1,
        static_cast<uint64_t>(Opcode::PUSH), 2,
        static_cast<uint64_t>(Opcode::ADD),  static_cast<uint64_t>(Opcode::HALT)};

    vm.load_program(program);

    // Test with limit equal to instruction count (should complete normally)
    vm.execute(4);
    assert(!vm.hit_instruction_limit_check());
    std::cout << "  ✓ Completed with limit == instruction count (4)" << std::endl;

    // Test with limit one less (should hit limit before HALT)
    vm.reset();
    vm.execute(3);
    assert(vm.hit_instruction_limit_check());
    std::cout << "  ✓ Hit limit with limit < instruction count (3 < 4)" << std::endl;

    std::cout << "\n✓ Test 4 passed\n" << std::endl;
}

void test_instruction_limit_with_profiling() {
    std::cout << "=== Test 5: Instruction Limit with Profiling ===" << std::endl;

    StackVM vm;

    // Program with loop
    std::vector<uint64_t> program = {
        static_cast<uint64_t>(Opcode::PUSH), 0,   // Counter
        static_cast<uint64_t>(Opcode::PUSH), 1,   // Loop: increment
        static_cast<uint64_t>(Opcode::ADD),       // add
        static_cast<uint64_t>(Opcode::DUP),       // duplicate for comparison
        static_cast<uint64_t>(Opcode::PUSH), 100, // compare with 100
        static_cast<uint64_t>(Opcode::LT),        // counter < 100?
        static_cast<uint64_t>(Opcode::JZ),   12,  // if false, jump to end
        static_cast<uint64_t>(Opcode::JMP),  3,   // else continue loop
        static_cast<uint64_t>(Opcode::HALT)};

    vm.load_program(program);
    vm.enable_profiling();
    vm.reset_profiling();

    // Execute with limit lower than loop iterations
    vm.execute(50);

    assert(vm.hit_instruction_limit_check());
    assert(vm.get_total_instructions() == 50);
    std::cout << "  ✓ Profiling tracked exactly " << vm.get_total_instructions()
              << " instructions before limit" << std::endl;

    std::cout << "\n✓ Test 5 passed\n" << std::endl;
}

int main() {
    std::cout << "=== VM Instruction Limit Tests ===" << std::endl;

    test_instruction_limit_infinite_loop();
    test_instruction_limit_normal_program();
    test_instruction_limit_no_limit();
    test_instruction_limit_exact_boundary();
    test_instruction_limit_with_profiling();

    std::cout << "\n=== All Instruction Limit Tests Passed ===" << std::endl;

    return 0;
}
