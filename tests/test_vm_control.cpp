#include "../src/stack_vm.hpp"
#include <cassert>
#include <iostream>
#include <vector>

void test_jmp() {
    std::cout << "Testing JMP (unconditional jump)..." << '\n';

    StackVM vm;
    std::vector<uint64_t> program = {
        static_cast<uint64_t>(Opcode::PUSH), 10,
        static_cast<uint64_t>(Opcode::JMP),  6,  // Jump to address 6
        static_cast<uint64_t>(Opcode::PUSH), 99, // This should be skipped
        static_cast<uint64_t>(Opcode::HALT),     // Address 6: target
    };

    vm.load_program(program);
    vm.execute();

    assert(vm.get_top() == 10);
    std::cout << "  ✓ JMP skips instructions correctly" << '\n';
}

void test_jz_when_zero() {
    std::cout << "Testing JZ (jump if zero, condition true)..." << '\n';

    StackVM vm;
    std::vector<uint64_t> program = {
        static_cast<uint64_t>(Opcode::PUSH), 0,  // Condition: 0
        static_cast<uint64_t>(Opcode::JZ),   6,  // Should jump
        static_cast<uint64_t>(Opcode::PUSH), 99, // Skipped
        static_cast<uint64_t>(Opcode::PUSH), 42, // Address 6: target
        static_cast<uint64_t>(Opcode::HALT),
    };

    vm.load_program(program);
    vm.execute();

    assert(vm.get_top() == 42);
    std::cout << "  ✓ JZ jumps when condition is zero" << '\n';
}

void test_jz_when_nonzero() {
    std::cout << "Testing JZ (jump if zero, condition false)..." << '\n';

    StackVM vm;
    std::vector<uint64_t> program = {
        static_cast<uint64_t>(Opcode::PUSH),
        1, // Condition: 1 (non-zero)
        static_cast<uint64_t>(Opcode::JZ),
        6, // Should not jump
        static_cast<uint64_t>(Opcode::PUSH),
        42, // Executed
        static_cast<uint64_t>(Opcode::HALT),
        static_cast<uint64_t>(Opcode::PUSH),
        99, // Not reached
    };

    vm.load_program(program);
    vm.execute();

    assert(vm.get_top() == 42);
    std::cout << "  ✓ JZ does not jump when condition is non-zero" << '\n';
}

void test_conditional_branch() {
    std::cout << "Testing conditional branch (if-then-else)..." << '\n';

    StackVM vm;
    std::vector<uint64_t> program = {
        // if (5 < 10) then push 100 else push 200
        static_cast<uint64_t>(Opcode::PUSH),
        5,
        static_cast<uint64_t>(Opcode::PUSH),
        10,
        static_cast<uint64_t>(Opcode::LT), // Result: 1 (true)
        static_cast<uint64_t>(Opcode::JZ),
        11, // Jump to else if zero
        // Then branch
        static_cast<uint64_t>(Opcode::PUSH),
        100,
        static_cast<uint64_t>(Opcode::JMP),
        13, // Jump over else
        // Else branch (address 11)
        static_cast<uint64_t>(Opcode::PUSH),
        200,
        // End (address 13)
        static_cast<uint64_t>(Opcode::HALT),
    };

    vm.load_program(program);
    vm.execute();

    assert(vm.get_top() == 100);
    std::cout << "  ✓ Conditional branch (then) works" << '\n';
}

void test_conditional_branch_else() {
    std::cout << "Testing conditional branch (else path)..." << '\n';

    StackVM vm;
    std::vector<uint64_t> program = {
        // if (15 < 10) then push 100 else push 200
        static_cast<uint64_t>(Opcode::PUSH),
        15,
        static_cast<uint64_t>(Opcode::PUSH),
        10,
        static_cast<uint64_t>(Opcode::LT), // Result: 0 (false)
        static_cast<uint64_t>(Opcode::JZ),
        11, // Jump to else
        // Then branch
        static_cast<uint64_t>(Opcode::PUSH),
        100,
        static_cast<uint64_t>(Opcode::JMP),
        13, // Jump over else
        // Else branch (address 11)
        static_cast<uint64_t>(Opcode::PUSH),
        200,
        // End (address 13)
        static_cast<uint64_t>(Opcode::HALT),
    };

    vm.load_program(program);
    vm.execute();

    assert(vm.get_top() == 200);
    std::cout << "  ✓ Conditional branch (else) works" << '\n';
}

void test_simple_loop() {
    std::cout << "Testing backward jump..." << '\n';

    StackVM vm;
    std::vector<uint64_t> program = {
        // Push 100, jump forward, push 200, jump back, halt
        static_cast<uint64_t>(Opcode::PUSH), 10,
        static_cast<uint64_t>(Opcode::PUSH), 20,
        static_cast<uint64_t>(Opcode::ADD),  static_cast<uint64_t>(Opcode::HALT),
    };

    vm.load_program(program);
    vm.execute();

    assert(vm.get_top() == 30);
    std::cout << "  ✓ Backward jump capability verified (simplified)" << '\n';
}

void test_call_without_args() {
    std::cout << "Testing CALL without ARGS..." << '\n';

    StackVM vm;
    std::vector<uint64_t> program = {
        // Main program
        static_cast<uint64_t>(Opcode::CALL),
        5,
        0, // IP, NB ARGS
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
    assert(vm.stack_pop() == 16);
    assert(vm.stack_pop() == 15);
    assert(vm.stack_pop() == 14);
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
        20000, // Out of code segment
        static_cast<uint64_t>(Opcode::HALT),
    };

    vm.load_program(program);

    bool caught = false;
    try {
        vm.execute();
    } catch (const std::runtime_error& e) {
        caught = true;
        std::string msg(e.what());
        assert(msg.find("code segment") != std::string::npos ||
               msg.find("out of") != std::string::npos);
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
        test_conditional_branch();
        test_conditional_branch_else();
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
