#include "../src/lisp_compiler.hpp"
#include "../src/lisp_parser.hpp"
#include "../src/stack_vm.hpp"
#include <cassert>
#include <iostream>

void test_compile_simple_interrupt() {
    std::cout << "Testing simple interrupt handler..." << '\n';

    std::string code = R"(
        (do
            (define-int 10 (print 9999))
            42)
    )";

    LispParser parser(code);
    auto ast = parser.parse();
    LispCompiler compiler;
    auto bytecode = compiler.compile(ast);

    StackVM vm;
    vm.load_program(bytecode);
    vm.execute();

    assert(vm.get_top() == 42);
    std::cout << "  ✓ Simple interrupt handler compiled successfully" << '\n';
}

void test_compile_interrupt_with_variable() {
    std::cout << "Testing interrupt handler with variable access..." << '\n';

    std::string code = R"(
        (do
            (define-var counter 0)
            (define-int 10 (set counter (+ counter 1)))
            counter)
    )";

    LispParser parser(code);
    auto ast = parser.parse();
    LispCompiler compiler;
    auto bytecode = compiler.compile(ast);

    StackVM vm;
    vm.load_program(bytecode);
    vm.execute();

    assert(vm.get_top() == 0); // Counter starts at 0
    std::cout << "  ✓ Interrupt handler with variable access compiled" << '\n';
}

void test_compile_multiple_interrupts() {
    std::cout << "Testing multiple interrupt handlers..." << '\n';

    std::string code = R"(
        (do
            (define-var counter1 0)
            (define-var counter2 0)
            (define-int 10 (set counter1 (+ counter1 1)))
            (define-int 12 (set counter2 (+ counter2 2)))
            (+ counter1 counter2))
    )";

    LispParser parser(code);
    auto ast = parser.parse();
    LispCompiler compiler;
    auto bytecode = compiler.compile(ast);

    StackVM vm;
    vm.load_program(bytecode);
    vm.execute();

    assert(vm.get_top() == 0); // Both counters start at 0
    std::cout << "  ✓ Multiple interrupt handlers compiled" << '\n';
}

void test_compile_interrupt_with_arithmetic() {
    std::cout << "Testing interrupt handler with arithmetic..." << '\n';

    std::string code = R"(
        (do
            (define-int 15 (* 7 8))
            100)
    )";

    LispParser parser(code);
    auto ast = parser.parse();
    LispCompiler compiler;
    auto bytecode = compiler.compile(ast);

    StackVM vm;
    vm.load_program(bytecode);
    vm.execute();

    assert(vm.get_top() == 100);
    std::cout << "  ✓ Interrupt with arithmetic compiled" << '\n';
}

void test_compile_interrupt_with_conditionals() {
    std::cout << "Testing interrupt handler with conditionals..." << '\n';

    std::string code = R"(
        (do
            (define-var flag 0)
            (define-int 20
                (if (= flag 0)
                    (set flag 1)
                    (set flag 0)))
            flag)
    )";

    LispParser parser(code);
    auto ast = parser.parse();
    LispCompiler compiler;
    auto bytecode = compiler.compile(ast);

    StackVM vm;
    vm.load_program(bytecode);
    vm.execute();

    assert(vm.get_top() == 0); // Flag starts at 0
    std::cout << "  ✓ Interrupt with conditionals compiled" << '\n';
}

void test_interrupt_number_validation() {
    std::cout << "Testing interrupt number validation..." << '\n';

    // Test signal number 0 (should fail)
    try {
        std::string code = "(define-int 0 (print 1))";
        LispParser parser(code);
        auto ast = parser.parse();
        LispCompiler compiler;
        compiler.compile(ast);
        assert(false); // Should have thrown
    } catch (const std::exception& e) {
        std::cout << "  ✓ Correctly rejected signal 0" << '\n';
    }

    // Test signal number 32 (should fail)
    try {
        std::string code = "(define-int 32 (print 1))";
        LispParser parser(code);
        auto ast = parser.parse();
        LispCompiler compiler;
        compiler.compile(ast);
        assert(false); // Should have thrown
    } catch (const std::exception& e) {
        std::cout << "  ✓ Correctly rejected signal 32" << '\n';
    }

    // Test valid signal numbers
    for (int sig = 1; sig <= 31; sig++) {
        std::string code = "(define-int " + std::to_string(sig) + " (print 1))";
        LispParser parser(code);
        auto ast = parser.parse();
        LispCompiler compiler;
        auto bytecode = compiler.compile(ast);
        // Should not throw
    }
    std::cout << "  ✓ All valid signal numbers (1-31) accepted" << '\n';
}

void test_bytecode_contains_iret() {
    std::cout << "Testing that bytecode contains IRET opcode..." << '\n';

    std::string code = R"(
        (do
            (define-int 10 42)
            0)
    )";

    LispParser parser(code);
    auto ast = parser.parse();
    LispCompiler compiler;
    auto bytecode = compiler.compile(ast);

    // Check that bytecode contains IRET opcode
    bool found_iret = false;
    for (size_t i = 0; i < bytecode.bytecode.size(); i++) {
        if (bytecode.bytecode[i] == static_cast<uint64_t>(Opcode::IRET)) {
            found_iret = true;
            break;
        }
    }

    assert(found_iret);
    std::cout << "  ✓ IRET opcode found in bytecode" << '\n';
}

void test_bytecode_contains_signal_reg() {
    std::cout << "Testing that bytecode contains SIGNAL_REG opcode..." << '\n';

    std::string code = R"(
        (do
            (define-int 10 42)
            0)
    )";

    LispParser parser(code);
    auto ast = parser.parse();
    LispCompiler compiler;
    auto bytecode = compiler.compile(ast);

    // Check that bytecode contains SIGNAL_REG opcode
    bool found_signal_reg = false;
    for (size_t i = 0; i < bytecode.bytecode.size(); i++) {
        if (bytecode.bytecode[i] == static_cast<uint64_t>(Opcode::SIGNAL_REG)) {
            found_signal_reg = true;
            break;
        }
    }

    assert(found_signal_reg);
    std::cout << "  ✓ SIGNAL_REG opcode found in bytecode" << '\n';
}

int main() {
    std::cout << "=== Compiler Interrupt Tests ===" << '\n' << '\n';

    try {
        test_compile_simple_interrupt();
        test_compile_interrupt_with_variable();
        test_compile_multiple_interrupts();
        test_compile_interrupt_with_arithmetic();
        test_compile_interrupt_with_conditionals();
        std::cout << '\n';

        test_interrupt_number_validation();
        test_bytecode_contains_iret();
        test_bytecode_contains_signal_reg();

        std::cout << "\n✓ All compiler interrupt tests passed!" << '\n';
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Test failed: " << e.what() << '\n';
        return 1;
    }
}
