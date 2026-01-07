#include "../src/stack_vm.hpp"
#include <cassert>
#include <iostream>
#include <vector>

void test_jmp() {
    std::cout << "Testing JMP (unconditional jump)..." << '\n';

    StackVM vm;
    std::vector<uint64_t> program = {
        static_cast<uint64_t>(Opcode::PUSH), 10, static_cast<uint64_t>(Opcode::JMP),  6,
        static_cast<uint64_t>(Opcode::PUSH), 99, static_cast<uint64_t>(Opcode::HALT),
    };

    const uint64_t initial_sp = vm.get_sp();

    vm.load_program(program);
    vm.execute();

    assert(vm.get_ip() == 7);
    assert(vm.get_sp() == initial_sp - 1);
    assert(vm.get_top() == 10);
    std::cout << "  ✓ JMP skips instructions correctly" << '\n';
}

void test_jz_when_zero() {
    std::cout << "Testing JZ (jump if zero, condition true)..." << '\n';

    StackVM vm;
    std::vector<uint64_t> program = {
        static_cast<uint64_t>(Opcode::PUSH), 0,  static_cast<uint64_t>(Opcode::JZ),   6,
        static_cast<uint64_t>(Opcode::PUSH), 99, static_cast<uint64_t>(Opcode::HALT),
    };

    const uint64_t initial_sp = vm.get_sp();

    vm.load_program(program);
    vm.execute();

    assert(vm.get_ip() == 7);
    assert(vm.get_sp() == initial_sp);

    std::cout << "  ✓ JZ jumps when condition is zero" << '\n';
}

void test_jz_when_nonzero() {
    std::cout << "Testing JZ (jump if zero, condition false)..." << '\n';

    StackVM vm;
    std::vector<uint64_t> program = {
        static_cast<uint64_t>(Opcode::PUSH),
        1,
        static_cast<uint64_t>(Opcode::JZ),
        6,
        static_cast<uint64_t>(Opcode::PUSH),
        42,
        static_cast<uint64_t>(Opcode::HALT),
        static_cast<uint64_t>(Opcode::PUSH),
        99,
    };

    const uint64_t initial_sp = vm.get_sp();

    vm.load_program(program);
    vm.execute();

    assert(vm.get_ip() == 7);
    assert(vm.get_top() == 42);
    assert(vm.get_sp() == initial_sp - 1);
    std::cout << "  ✓ JZ does not jump when condition is non-zero" << '\n';
}

void test_simple_loop() {
    std::cout << "Testing backward jump..." << '\n';

    StackVM vm;
    std::vector<uint64_t> program = {
        static_cast<uint64_t>(Opcode::PUSH),
        10,
        static_cast<uint64_t>(Opcode::JMP),
        6,
        static_cast<uint64_t>(Opcode::PUSH),
        20,
        static_cast<uint64_t>(Opcode::HALT),
        static_cast<uint64_t>(Opcode::JMP),
        4,
    };

    const uint64_t initial_sp = vm.get_sp();

    vm.load_program(program);
    vm.execute();

    assert(vm.get_ip() == 7);
    assert(vm.get_sp() == initial_sp - 1);
    assert(vm.get_top() == 10);

    vm.execute();
    assert(vm.get_ip() == 7);
    assert(vm.get_sp() == initial_sp - 2);
    assert(vm.get_top() == 20);

    std::cout << "  ✓ Backward jump capability verified (simplified)" << '\n';
}

void test_call_without_args() {
    std::cout << "Testing CALL without ARGS..." << '\n';

    StackVM vm;
    std::vector<uint64_t> program = {
        static_cast<uint64_t>(Opcode::CALL),
        5,
        0,
        static_cast<uint64_t>(Opcode::PUSH),
        10,
        // Fn 1
        static_cast<uint64_t>(Opcode::HALT),
    };

    vm.load_program(program);
    const uint64_t STARTING_SP = vm.get_sp();
    vm.execute();

    assert(vm.get_sp() == STARTING_SP - 1);
    assert(vm.get_top() == 3); // First function IP
    assert(vm.get_ip() == 6);
    std::cout << "  ✓ CALL/RET: function" << '\n';
}

void test_call_with_args() {
    std::cout << "Testing CALL with ARGS..." << '\n';

    StackVM vm;
    std::vector<uint64_t> program = {
        // Main program
        static_cast<uint64_t>(Opcode::PUSH),
        14,
        static_cast<uint64_t>(Opcode::PUSH),
        15,
        static_cast<uint64_t>(Opcode::PUSH),
        16,
        static_cast<uint64_t>(Opcode::CALL),
        11,
        3, // IP, NB ARGS
        static_cast<uint64_t>(Opcode::PUSH),
        10,

        // Fn 1
        static_cast<uint64_t>(Opcode::HALT),
    };

    vm.load_program(program);
    const uint64_t STARTING_SP = vm.get_sp();
    vm.execute();

    assert(vm.get_sp() == STARTING_SP - 7);
    assert(vm.stack_pop() == 14);
    assert(vm.stack_pop() == 15);
    assert(vm.stack_pop() == 16);
    assert(vm.stack_pop() == 9); // IP In FN 1
    assert(vm.stack_pop() == 16);
    assert(vm.stack_pop() == 15);
    assert(vm.stack_pop() == 14);
    assert(vm.get_ip() == 12);
    std::cout << "  ✓ CALL/RET: function" << '\n';
}

void test_call_ret_enter_leave() {
    std::cout << "Testing CALL and RET with ENTER and LEAVE..." << '\n';

    StackVM vm;
    std::vector<uint64_t> program = {
        // Main program
        static_cast<uint64_t>(Opcode::PUSH), 10,   // Argument
        static_cast<uint64_t>(Opcode::CALL), 6, 1, // IP, NB_ARGS
        static_cast<uint64_t>(Opcode::HALT),

        // Fn 1
        static_cast<uint64_t>(Opcode::ENTER), 0, // Temps
        static_cast<uint64_t>(Opcode::PUSH), 0, static_cast<uint64_t>(Opcode::BP_LOAD),
        static_cast<uint64_t>(Opcode::PUSH), 2, static_cast<uint64_t>(Opcode::MUL),
        static_cast<uint64_t>(Opcode::LEAVE), 0, // Temps
        static_cast<uint64_t>(Opcode::RET), 1    // Args
    };

    vm.load_program(program);
    const uint64_t STARTING_SP = vm.get_sp();
    vm.execute();

    assert(vm.get_ip() == 6);
    assert(vm.get_sp() == STARTING_SP - 1);
    assert(vm.get_top() == 20);
    std::cout << "  ✓ CALL/RET: function doubles 10 -> 20" << '\n';
}

void test_nested_calls_enter_leave() {
    std::cout << "Testing nested function calls with ENTER and LEAVE..." << '\n';

    StackVM vm;
    std::vector<uint64_t> program = {
        // Main
        static_cast<uint64_t>(Opcode::PUSH), 5,    // arg 1
        static_cast<uint64_t>(Opcode::CALL), 6, 1, // addresse et le nombre d'arguments
        static_cast<uint64_t>(Opcode::HALT),

        // Fn 1
        static_cast<uint64_t>(Opcode::ENTER), 0, // args & temps
        static_cast<uint64_t>(Opcode::PUSH), 0,
        static_cast<uint64_t>(Opcode::BP_LOAD), // get argument
        static_cast<uint64_t>(Opcode::PUSH), 10, static_cast<uint64_t>(Opcode::ADD),
        static_cast<uint64_t>(Opcode::CALL), 21, 1, static_cast<uint64_t>(Opcode::LEAVE), 0,
        static_cast<uint64_t>(Opcode::RET), 1, // nb args to pop automatiquement

        // Fn 2
        static_cast<uint64_t>(Opcode::ENTER), 0, static_cast<uint64_t>(Opcode::PUSH), 2,
        static_cast<uint64_t>(Opcode::PUSH), 0,
        static_cast<uint64_t>(Opcode::BP_LOAD), // Push first argument
        static_cast<uint64_t>(Opcode::MUL), static_cast<uint64_t>(Opcode::LEAVE), 0,
        static_cast<uint64_t>(Opcode::RET), 1 // nb args to pop automatiquement
    };

    vm.load_program(program);
    const uint64_t STARTING_SP = vm.get_sp();
    vm.execute();

    // (5 + 10) * 2 = 30
    assert(vm.get_ip() == 6);
    assert(vm.get_sp() == STARTING_SP - 1);
    assert(vm.get_top() == 30);
    std::cout << "  ✓ Nested calls: (5 + 10) * 2 = 30" << '\n';
}

void test_jump_bounds_check() {
    std::cout << "Testing jump bounds checking..." << '\n';

    StackVM vm;
    std::vector<uint64_t> program = {
        static_cast<uint64_t>(Opcode::JMP),
        70000, // Out of bounds (> MEMORY_SIZE = 65536)
        static_cast<uint64_t>(Opcode::HALT),
    };

    vm.load_program(program);

    bool caught = false;
    try {
        vm.execute();
    } catch (const std::runtime_error& e) {
        caught = true;
        std::string msg(e.what());
        assert(msg.find("out of") != std::string::npos || msg.find("bounds") != std::string::npos);
    }

    assert(caught);
    std::cout << "  ✓ Jump bounds checking works" << '\n';
}

int main() {
    std::cout << "=== VM Control Flow Tests ===" << '\n';

    try {
        test_jmp();
        test_jz_when_zero();
        test_jz_when_nonzero();
        test_simple_loop();
        test_jump_bounds_check();
        test_call_without_args();
        test_call_with_args();
        test_call_ret_enter_leave();
        test_nested_calls_enter_leave();

        std::cout << "\n✓ All control flow tests passed!" << '\n';
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Test failed: " << e.what() << '\n';
        return 1;
    }
}
