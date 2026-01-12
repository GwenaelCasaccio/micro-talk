#include "../src/stack_vm.hpp"
#include <cassert>
#include <iostream>
#include <vector>

// Check if bounds checking is enabled (matches stack_vm.hpp configuration)
#ifndef MICRO_TALK_BOUNDS_CHECKS
#ifdef NDEBUG
#define MICRO_TALK_BOUNDS_CHECKS 0
#else
#define MICRO_TALK_BOUNDS_CHECKS 1
#endif
#endif

void test_push() {
    std::cout << "Testing PUSH..." << '\n';

    StackVM vm;
    std::vector<uint64_t> program = {static_cast<uint64_t>(Opcode::PUSH), 42,
                                     static_cast<uint64_t>(Opcode::HALT)};

    const uint64_t initial_sp = vm.get_sp();

    vm.load_program(program);
    vm.execute();

    assert(vm.get_sp() == initial_sp - 1);
    assert(vm.get_top() == 42);
    std::cout << "  ✓ PUSH works correctly" << '\n';
}

void test_pop() {
    std::cout << "Testing POP..." << '\n';

    StackVM vm;

    std::vector<uint64_t> program = {
        static_cast<uint64_t>(Opcode::PUSH), 42,
        static_cast<uint64_t>(Opcode::HALT), static_cast<uint64_t>(Opcode::POP),
        static_cast<uint64_t>(Opcode::HALT),
    };

    const uint64_t initial_sp = vm.get_sp();

    vm.load_program(program);
    vm.execute();
    assert(vm.get_ip() == 3);

    vm.execute();

    assert(vm.get_ip() == 5);
    assert(vm.get_sp() == initial_sp);

    std::cout << "  ✓ POP works correctly" << '\n';
}

void test_push_pop() {
    std::cout << "Testing PUSH and POP..." << '\n';

    StackVM vm;
    std::vector<uint64_t> program = {
        static_cast<uint64_t>(Opcode::PUSH), 42,
        static_cast<uint64_t>(Opcode::PUSH), 100,
        static_cast<uint64_t>(Opcode::POP),  static_cast<uint64_t>(Opcode::HALT)};

    const uint64_t initial_sp = vm.get_sp();
    vm.load_program(program);
    vm.execute();

    assert(vm.get_sp() == initial_sp - 1);
    assert(vm.get_top() == 42);
    assert(vm.get_ip() == 6);
    std::cout << "  ✓ PUSH/POP works correctly" << '\n';
}

void test_dup() {
    std::cout << "Testing DUP..." << '\n';

    StackVM vm;
    std::vector<uint64_t> program = {
        static_cast<uint64_t>(Opcode::PUSH), 123,
        static_cast<uint64_t>(Opcode::DUP),  static_cast<uint64_t>(Opcode::HALT),
        static_cast<uint64_t>(Opcode::POP),  static_cast<uint64_t>(Opcode::HALT)};

    const uint64_t initial_sp = vm.get_sp();
    vm.load_program(program);
    vm.execute();

    assert(vm.get_top() == 123);
    assert(vm.get_sp() == initial_sp - 2);
    assert(vm.get_ip() == 4);

    vm.execute();
    assert(vm.get_top() == 123);
    assert(vm.get_sp() == initial_sp - 1);
    assert(vm.get_ip() == 6);

    std::cout << "  ✓ DUP works correctly" << '\n';
}

void test_stack_underflow() {
    std::cout << "Testing stack underflow detection..." << '\n';

#if MICRO_TALK_BOUNDS_CHECKS
    // Only run this test when bounds checking is enabled
    StackVM vm;
    std::vector<uint64_t> program = {static_cast<uint64_t>(Opcode::POP),
                                     static_cast<uint64_t>(Opcode::HALT)};

    vm.load_program(program);

    bool caught = false;
    try {
        vm.execute();
    } catch (const std::runtime_error& e) {
        caught = true;
        std::string msg(e.what());
        assert(msg.find("underflow") != std::string::npos);
    }

    assert(caught);
    std::cout << "  ✓ Stack underflow detected correctly" << '\n';
#else
    // With bounds checking disabled, skip test entirely (would cause undefined behavior)
    std::cout << "  ⊘ Stack underflow test skipped (bounds checking disabled)" << '\n';
#endif
}

int main() {
    std::cout << "=== VM Stack Operations Tests ===" << '\n';

    try {
        test_push();
        test_pop();
        test_push_pop();
        test_dup();
        test_stack_underflow();

        std::cout << "\n✓ All stack operation tests passed!" << '\n';
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Test failed: " << e.what() << '\n';
        return 1;
    }
}
