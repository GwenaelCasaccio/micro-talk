#include "../src/stack_vm.hpp"
#include <cassert>
#include <iostream>
#include <vector>

// Arithmetic operations

void test_add() {
    std::cout << "Testing ADD..." << '\n';

    StackVM vm;
    std::vector<uint64_t> program = {
        static_cast<uint64_t>(Opcode::PUSH), 10,
        static_cast<uint64_t>(Opcode::PUSH), 20,
        static_cast<uint64_t>(Opcode::ADD),  static_cast<uint64_t>(Opcode::HALT)};

    vm.load_program(program);
    vm.execute();

    assert(vm.get_top() == 30);
    std::cout << "  ✓ ADD: 10 + 20 = 30" << '\n';
}

void test_sub() {
    std::cout << "Testing SUB..." << '\n';

    StackVM vm;
    std::vector<uint64_t> program = {
        static_cast<uint64_t>(Opcode::PUSH), 50,
        static_cast<uint64_t>(Opcode::PUSH), 20,
        static_cast<uint64_t>(Opcode::SUB),  static_cast<uint64_t>(Opcode::HALT)};

    vm.load_program(program);
    vm.execute();

    assert(vm.get_top() == 30);
    std::cout << "  ✓ SUB: 50 - 20 = 30" << '\n';
}

void test_mul() {
    std::cout << "Testing MUL..." << '\n';

    StackVM vm;
    std::vector<uint64_t> program = {
        static_cast<uint64_t>(Opcode::PUSH), 6,
        static_cast<uint64_t>(Opcode::PUSH), 7,
        static_cast<uint64_t>(Opcode::MUL),  static_cast<uint64_t>(Opcode::HALT)};

    vm.load_program(program);
    vm.execute();

    assert(vm.get_top() == 42);
    std::cout << "  ✓ MUL: 6 * 7 = 42" << '\n';
}

void test_div() {
    std::cout << "Testing DIV..." << '\n';

    StackVM vm;
    std::vector<uint64_t> program = {
        static_cast<uint64_t>(Opcode::PUSH), 100,
        static_cast<uint64_t>(Opcode::PUSH), 5,
        static_cast<uint64_t>(Opcode::DIV),  static_cast<uint64_t>(Opcode::HALT)};

    vm.load_program(program);
    vm.execute();

    assert(vm.get_top() == 20);
    std::cout << "  ✓ DIV: 100 / 5 = 20" << '\n';
}

void test_div_by_zero() {
    std::cout << "Testing division by zero detection..." << '\n';

    StackVM vm;
    std::vector<uint64_t> program = {
        static_cast<uint64_t>(Opcode::PUSH), 42,
        static_cast<uint64_t>(Opcode::PUSH), 0,
        static_cast<uint64_t>(Opcode::DIV),  static_cast<uint64_t>(Opcode::HALT)};

    vm.load_program(program);

    bool caught = false;
    try {
        vm.execute();
    } catch (const std::runtime_error& e) {
        caught = true;
        std::string msg(e.what());
        assert(msg.find("Division by zero") != std::string::npos);
    }

    assert(caught);
    std::cout << "  ✓ Division by zero detected" << '\n';
}

void test_mod() {
    std::cout << "Testing MOD..." << '\n';

    StackVM vm;
    std::vector<uint64_t> program = {
        static_cast<uint64_t>(Opcode::PUSH), 17,
        static_cast<uint64_t>(Opcode::PUSH), 5,
        static_cast<uint64_t>(Opcode::MOD),  static_cast<uint64_t>(Opcode::HALT)};

    vm.load_program(program);
    vm.execute();

    assert(vm.get_top() == 2);
    std::cout << "  ✓ MOD: 17 % 5 = 2" << '\n';
}

void test_mod_by_zero() {
    std::cout << "Testing modulo by zero detection..." << '\n';

    StackVM vm;
    std::vector<uint64_t> program = {
        static_cast<uint64_t>(Opcode::PUSH), 42,
        static_cast<uint64_t>(Opcode::PUSH), 0,
        static_cast<uint64_t>(Opcode::MOD),  static_cast<uint64_t>(Opcode::HALT)};

    vm.load_program(program);

    bool caught = false;
    try {
        vm.execute();
    } catch (const std::runtime_error& e) {
        caught = true;
        std::string msg(e.what());
        assert(msg.find("Modulo by zero") != std::string::npos);
    }

    assert(caught);
    std::cout << "  ✓ Modulo by zero detected" << '\n';
}

// Comparison operations

void test_eq_true() {
    std::cout << "Testing EQ (equal)..." << '\n';

    StackVM vm;
    std::vector<uint64_t> program = {
        static_cast<uint64_t>(Opcode::PUSH), 42,
        static_cast<uint64_t>(Opcode::PUSH), 42,
        static_cast<uint64_t>(Opcode::EQ),   static_cast<uint64_t>(Opcode::HALT)};

    vm.load_program(program);
    vm.execute();

    assert(vm.get_top() == 1);
    std::cout << "  ✓ EQ: 42 == 42 -> 1 (true)" << '\n';
}

void test_eq_false() {
    std::cout << "Testing EQ (not equal)..." << '\n';

    StackVM vm;
    std::vector<uint64_t> program = {
        static_cast<uint64_t>(Opcode::PUSH), 42,
        static_cast<uint64_t>(Opcode::PUSH), 100,
        static_cast<uint64_t>(Opcode::EQ),   static_cast<uint64_t>(Opcode::HALT)};

    vm.load_program(program);
    vm.execute();

    assert(vm.get_top() == 0);
    std::cout << "  ✓ EQ: 42 == 100 -> 0 (false)" << '\n';
}

void test_lt_true() {
    std::cout << "Testing LT (less than, true)..." << '\n';

    StackVM vm;
    std::vector<uint64_t> program = {
        static_cast<uint64_t>(Opcode::PUSH), 10,
        static_cast<uint64_t>(Opcode::PUSH), 20,
        static_cast<uint64_t>(Opcode::LT),   static_cast<uint64_t>(Opcode::HALT)};

    vm.load_program(program);
    vm.execute();

    assert(vm.get_top() == 1);
    std::cout << "  ✓ LT: 10 < 20 -> 1 (true)" << '\n';
}

void test_lt_false() {
    std::cout << "Testing LT (less than, false)..." << '\n';

    StackVM vm;
    std::vector<uint64_t> program = {
        static_cast<uint64_t>(Opcode::PUSH), 20,
        static_cast<uint64_t>(Opcode::PUSH), 10,
        static_cast<uint64_t>(Opcode::LT),   static_cast<uint64_t>(Opcode::HALT)};

    vm.load_program(program);
    vm.execute();

    assert(vm.get_top() == 0);
    std::cout << "  ✓ LT: 20 < 10 -> 0 (false)" << '\n';
}

void test_gt_true() {
    std::cout << "Testing GT (greater than, true)..." << '\n';

    StackVM vm;
    std::vector<uint64_t> program = {
        static_cast<uint64_t>(Opcode::PUSH), 30,
        static_cast<uint64_t>(Opcode::PUSH), 10,
        static_cast<uint64_t>(Opcode::GT),   static_cast<uint64_t>(Opcode::HALT)};

    vm.load_program(program);
    vm.execute();

    assert(vm.get_top() == 1);
    std::cout << "  ✓ GT: 30 > 10 -> 1 (true)" << '\n';
}

void test_gt_false() {
    std::cout << "Testing GT (greater than, false)..." << '\n';

    StackVM vm;
    std::vector<uint64_t> program = {
        static_cast<uint64_t>(Opcode::PUSH), 5,
        static_cast<uint64_t>(Opcode::PUSH), 10,
        static_cast<uint64_t>(Opcode::GT),   static_cast<uint64_t>(Opcode::HALT)};

    vm.load_program(program);
    vm.execute();

    assert(vm.get_top() == 0);
    std::cout << "  ✓ GT: 5 > 10 -> 0 (false)" << '\n';
}

// Bitwise operations

void test_and() {
    std::cout << "Testing AND..." << '\n';

    StackVM vm;
    std::vector<uint64_t> program = {
        static_cast<uint64_t>(Opcode::PUSH), 0b1100,
        static_cast<uint64_t>(Opcode::PUSH), 0b1010,
        static_cast<uint64_t>(Opcode::AND),  static_cast<uint64_t>(Opcode::HALT)};

    vm.load_program(program);
    vm.execute();

    assert(vm.get_top() == 0b1000);
    std::cout << "  ✓ AND: 0b1100 & 0b1010 = 0b1000" << '\n';
}

void test_or() {
    std::cout << "Testing OR..." << '\n';

    StackVM vm;
    std::vector<uint64_t> program = {
        static_cast<uint64_t>(Opcode::PUSH), 0b1100,
        static_cast<uint64_t>(Opcode::PUSH), 0b1010,
        static_cast<uint64_t>(Opcode::OR),   static_cast<uint64_t>(Opcode::HALT)};

    vm.load_program(program);
    vm.execute();

    assert(vm.get_top() == 0b1110);
    std::cout << "  ✓ OR: 0b1100 | 0b1010 = 0b1110" << '\n';
}

void test_xor() {
    std::cout << "Testing XOR..." << '\n';

    StackVM vm;
    std::vector<uint64_t> program = {
        static_cast<uint64_t>(Opcode::PUSH), 0b1100,
        static_cast<uint64_t>(Opcode::PUSH), 0b1010,
        static_cast<uint64_t>(Opcode::XOR),  static_cast<uint64_t>(Opcode::HALT)};

    vm.load_program(program);
    vm.execute();

    assert(vm.get_top() == 0b0110);
    std::cout << "  ✓ XOR: 0b1100 ^ 0b1010 = 0b0110" << '\n';
}

void test_shl() {
    std::cout << "Testing SHL (shift left)..." << '\n';

    StackVM vm;
    std::vector<uint64_t> program = {
        static_cast<uint64_t>(Opcode::PUSH), 5,
        static_cast<uint64_t>(Opcode::PUSH), 2,
        static_cast<uint64_t>(Opcode::SHL),  static_cast<uint64_t>(Opcode::HALT)};

    vm.load_program(program);
    vm.execute();

    assert(vm.get_top() == 20);
    std::cout << "  ✓ SHL: 5 << 2 = 20" << '\n';
}

void test_shr() {
    std::cout << "Testing SHR (logical shift right)..." << '\n';

    StackVM vm;
    std::vector<uint64_t> program = {
        static_cast<uint64_t>(Opcode::PUSH), 20,
        static_cast<uint64_t>(Opcode::PUSH), 2,
        static_cast<uint64_t>(Opcode::SHR),  static_cast<uint64_t>(Opcode::HALT)};

    vm.load_program(program);
    vm.execute();

    assert(vm.get_top() == 5);
    std::cout << "  ✓ SHR: 20 >> 2 = 5" << '\n';
}

void test_ashr() {
    std::cout << "Testing ASHR (arithmetic shift right)..." << '\n';

    // Test with positive number
    StackVM vm1;
    std::vector<uint64_t> program1 = {
        static_cast<uint64_t>(Opcode::PUSH), 20,
        static_cast<uint64_t>(Opcode::PUSH), 2,
        static_cast<uint64_t>(Opcode::ASHR), static_cast<uint64_t>(Opcode::HALT)};

    vm1.load_program(program1);
    vm1.execute();

    assert(vm1.get_top() == 5);
    std::cout << "  ✓ ASHR: 20 >> 2 = 5 (positive)" << '\n';

    // Test with negative number (sign extension)
    StackVM vm2;
    std::vector<uint64_t> program2 = {
        static_cast<uint64_t>(Opcode::PUSH), static_cast<uint64_t>(-8),
        static_cast<uint64_t>(Opcode::PUSH), 1,
        static_cast<uint64_t>(Opcode::ASHR), static_cast<uint64_t>(Opcode::HALT)};

    vm2.load_program(program2);
    vm2.execute();

    assert(static_cast<int64_t>(vm2.get_top()) == -4);
    std::cout << "  ✓ ASHR: -8 >> 1 = -4 (sign extension)" << '\n';
}

// Complex ALU operations

void test_complex_expression() {
    std::cout << "Testing complex expression: (10 + 5) * 2..." << '\n';

    StackVM vm;
    std::vector<uint64_t> program = {static_cast<uint64_t>(Opcode::PUSH),
                                     10,
                                     static_cast<uint64_t>(Opcode::PUSH),
                                     5,
                                     static_cast<uint64_t>(Opcode::ADD),
                                     static_cast<uint64_t>(Opcode::PUSH),
                                     2,
                                     static_cast<uint64_t>(Opcode::MUL),
                                     static_cast<uint64_t>(Opcode::HALT)};

    vm.load_program(program);
    vm.execute();

    assert(vm.get_top() == 30);
    std::cout << "  ✓ (10 + 5) * 2 = 30" << '\n';
}

void test_tagging_operations() {
    std::cout << "Testing tagged integer operations..." << '\n';

    // Tag an integer: (value << 3) | 1
    StackVM vm;
    std::vector<uint64_t> program = {static_cast<uint64_t>(Opcode::PUSH),
                                     42,
                                     static_cast<uint64_t>(Opcode::PUSH),
                                     3,
                                     static_cast<uint64_t>(Opcode::SHL),
                                     static_cast<uint64_t>(Opcode::PUSH),
                                     1,
                                     static_cast<uint64_t>(Opcode::OR),
                                     static_cast<uint64_t>(Opcode::HALT)};

    vm.load_program(program);
    vm.execute();

    assert(vm.get_top() == 337); // (42 << 3) | 1
    std::cout << "  ✓ Tag int: (42 << 3) | 1 = 337" << '\n';
}

int main() {
    std::cout << "=== VM ALU Tests ===" << '\n';

    try {
        // Arithmetic
        test_add();
        test_sub();
        test_mul();
        test_div();
        test_div_by_zero();
        test_mod();
        test_mod_by_zero();

        std::cout << '\n';

        // Comparison
        test_eq_true();
        test_eq_false();
        test_lt_true();
        test_lt_false();
        test_gt_true();
        test_gt_false();

        std::cout << '\n';

        // Bitwise
        test_and();
        test_or();
        test_xor();
        test_shl();
        test_shr();
        test_ashr();

        std::cout << '\n';

        // Complex
        test_complex_expression();
        test_tagging_operations();

        std::cout << "\n✓ All ALU tests passed!" << '\n';
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Test failed: " << e.what() << '\n';
        return 1;
    }
}
