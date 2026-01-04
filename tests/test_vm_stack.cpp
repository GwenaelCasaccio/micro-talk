#include "../src/stack_vm.hpp"
#include <cassert>
#include <iostream>
#include <vector>

void test_push_pop() {
    std::cout << "Testing PUSH and POP..." << '\n';

    StackVM vm;
    std::vector<uint64_t> program = {
        static_cast<uint64_t>(Opcode::PUSH), 42,
        static_cast<uint64_t>(Opcode::PUSH), 100,
        static_cast<uint64_t>(Opcode::POP),  static_cast<uint64_t>(Opcode::HALT)};

    vm.load_program(program);
    vm.execute();

    assert(vm.get_top() == 42);
    std::cout << "  ✓ PUSH/POP works correctly" << '\n';
}

void test_dup() {
    std::cout << "Testing DUP..." << '\n';

    StackVM vm;
    std::vector<uint64_t> program = {static_cast<uint64_t>(Opcode::PUSH), 123,
                                     static_cast<uint64_t>(Opcode::DUP),
                                     static_cast<uint64_t>(Opcode::HALT)};

    vm.load_program(program);
    vm.execute();

    assert(vm.get_top() == 123);
    // Pop once and check if duplicate is there
    program = {static_cast<uint64_t>(Opcode::PUSH), 123, static_cast<uint64_t>(Opcode::DUP),
               static_cast<uint64_t>(Opcode::POP), static_cast<uint64_t>(Opcode::HALT)};

    StackVM vm2;
    vm2.load_program(program);
    vm2.execute();
    assert(vm2.get_top() == 123);

    std::cout << "  ✓ DUP works correctly" << '\n';
}

void test_swap() {
    std::cout << "Testing SWAP..." << '\n';

    StackVM vm;
    std::vector<uint64_t> program = {
        static_cast<uint64_t>(Opcode::PUSH), 10,
        static_cast<uint64_t>(Opcode::PUSH), 20,
        static_cast<uint64_t>(Opcode::SWAP), static_cast<uint64_t>(Opcode::HALT)};

    vm.load_program(program);
    vm.execute();

    // After SWAP, 10 should be on top
    assert(vm.get_top() == 10);
    std::cout << "  ✓ SWAP works correctly" << '\n';
}

void test_stack_underflow() {
    std::cout << "Testing stack underflow detection..." << '\n';

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
}

void test_multiple_operations() {
    std::cout << "Testing complex stack operations..." << '\n';

    StackVM vm;
    std::vector<uint64_t> program = {
        static_cast<uint64_t>(Opcode::PUSH), 1, static_cast<uint64_t>(Opcode::PUSH), 2,
        static_cast<uint64_t>(Opcode::PUSH), 3,
        static_cast<uint64_t>(Opcode::DUP),  // Stack: 1, 2, 3, 3
        static_cast<uint64_t>(Opcode::SWAP), // Stack: 1, 2, 3, 3 -> 1, 2, 3, 3
        static_cast<uint64_t>(Opcode::POP),  // Stack: 1, 2, 3
        static_cast<uint64_t>(Opcode::HALT)};

    vm.load_program(program);
    vm.execute();

    assert(vm.get_top() == 3);
    std::cout << "  ✓ Complex stack operations work correctly" << '\n';
}

int main() {
    std::cout << "=== VM Stack Operations Tests ===" << '\n';

    try {
        test_push_pop();
        test_dup();
        test_swap();
        test_stack_underflow();
        test_multiple_operations();

        std::cout << "\n✓ All stack operation tests passed!" << '\n';
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Test failed: " << e.what() << '\n';
        return 1;
    }
}
