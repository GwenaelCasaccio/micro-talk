#include "../src/lisp_compiler.hpp"
#include "../src/lisp_parser.hpp"
#include "../src/stack_vm.hpp"
#include <cassert>
#include <iostream>

void test_compile_number() {
    std::cout << "Testing number compilation..." << '\n';

    LispParser parser("42");
    auto ast = parser.parse();
    LispCompiler compiler;
    auto bytecode = compiler.compile(ast);

    StackVM vm;
    vm.load_program(bytecode);
    vm.execute();

    assert(vm.get_top() == 42);
    std::cout << "  ✓ Number literal: 42" << '\n';
}

void test_compile_arithmetic() {
    std::cout << "Testing arithmetic compilation..." << '\n';

    // Addition
    {
        LispParser parser("(+ 10 20)");
        auto ast = parser.parse();
        LispCompiler compiler;
        auto bytecode = compiler.compile(ast);

        StackVM vm;
        vm.load_program(bytecode);
        vm.execute();

        assert(vm.get_top() == 30);
        std::cout << "  ✓ Addition: (+ 10 20) = 30" << '\n';
    }

    // Subtraction
    {
        LispParser parser("(- 50 20)");
        auto ast = parser.parse();
        LispCompiler compiler;
        auto bytecode = compiler.compile(ast);

        StackVM vm;
        vm.load_program(bytecode);
        vm.execute();

        assert(vm.get_top() == 30);
        std::cout << "  ✓ Subtraction: (- 50 20) = 30" << '\n';
    }

    // Multiplication
    {
        LispParser parser("(* 6 7)");
        auto ast = parser.parse();
        LispCompiler compiler;
        auto bytecode = compiler.compile(ast);

        StackVM vm;
        vm.load_program(bytecode);
        vm.execute();

        assert(vm.get_top() == 42);
        std::cout << "  ✓ Multiplication: (* 6 7) = 42" << '\n';
    }

    // Division
    {
        LispParser parser("(/ 100 5)");
        auto ast = parser.parse();
        LispCompiler compiler;
        auto bytecode = compiler.compile(ast);

        StackVM vm;
        vm.load_program(bytecode);
        vm.execute();

        assert(vm.get_top() == 20);
        std::cout << "  ✓ Division: (/ 100 5) = 20" << '\n';
    }

    // Modulo
    {
        LispParser parser("(% 17 5)");
        auto ast = parser.parse();
        LispCompiler compiler;
        auto bytecode = compiler.compile(ast);

        StackVM vm;
        vm.load_program(bytecode);
        vm.execute();

        assert(vm.get_top() == 2);
        std::cout << "  ✓ Modulo: (% 17 5) = 2" << '\n';
    }

    // Multi-argument addition
    {
        LispParser parser("(+ 1 2 3 4 5)");
        auto ast = parser.parse();
        LispCompiler compiler;
        auto bytecode = compiler.compile(ast);

        StackVM vm;
        vm.load_program(bytecode);
        vm.execute();

        assert(vm.get_top() == 15);
        std::cout << "  ✓ Multi-arg addition: (+ 1 2 3 4 5) = 15" << '\n';
    }
}

void test_compile_comparison() {
    std::cout << "Testing comparison compilation..." << '\n';

    // Equal (true)
    {
        LispParser parser("(= 42 42)");
        auto ast = parser.parse();
        LispCompiler compiler;
        auto bytecode = compiler.compile(ast);

        StackVM vm;
        vm.load_program(bytecode);
        vm.execute();

        assert(vm.get_top() == 1);
        std::cout << "  ✓ Equal (true): (= 42 42) = 1" << '\n';
    }

    // Equal (false)
    {
        LispParser parser("(= 42 100)");
        auto ast = parser.parse();
        LispCompiler compiler;
        auto bytecode = compiler.compile(ast);

        StackVM vm;
        vm.load_program(bytecode);
        vm.execute();

        assert(vm.get_top() == 0);
        std::cout << "  ✓ Equal (false): (= 42 100) = 0" << '\n';
    }

    // Less than (true)
    {
        LispParser parser("(< 10 20)");
        auto ast = parser.parse();
        LispCompiler compiler;
        auto bytecode = compiler.compile(ast);

        StackVM vm;
        vm.load_program(bytecode);
        vm.execute();

        assert(vm.get_top() == 1);
        std::cout << "  ✓ Less than (true): (< 10 20) = 1" << '\n';
    }

    // Greater than (false)
    {
        LispParser parser("(> 5 10)");
        auto ast = parser.parse();
        LispCompiler compiler;
        auto bytecode = compiler.compile(ast);

        StackVM vm;
        vm.load_program(bytecode);
        vm.execute();

        assert(vm.get_top() == 0);
        std::cout << "  ✓ Greater than (false): (> 5 10) = 0" << '\n';
    }
}

void test_compile_nested() {
    std::cout << "Testing nested expression compilation..." << '\n';

    {
        LispParser parser("(+ (* 2 3) 4)");
        auto ast = parser.parse();
        LispCompiler compiler;
        auto bytecode = compiler.compile(ast);

        StackVM vm;
        vm.load_program(bytecode);
        vm.execute();

        assert(vm.get_top() == 10);
        std::cout << "  ✓ Nested: (+ (* 2 3) 4) = 10" << '\n';
    }

    {
        LispParser parser("(* (+ 2 3) (- 10 4))");
        auto ast = parser.parse();
        LispCompiler compiler;
        auto bytecode = compiler.compile(ast);

        StackVM vm;
        vm.load_program(bytecode);
        vm.execute();

        assert(vm.get_top() == 30);
        std::cout << "  ✓ Complex nested: (* (+ 2 3) (- 10 4)) = 30" << '\n';
    }

    {
        LispParser parser("(+ (* 2 (+ 3 4)) (- 10 5))");
        auto ast = parser.parse();
        LispCompiler compiler;
        auto bytecode = compiler.compile(ast);

        StackVM vm;
        vm.load_program(bytecode);
        vm.execute();

        assert(vm.get_top() == 19);
        std::cout << "  ✓ Deep nested: (+ (* 2 (+ 3 4)) (- 10 5)) = 19" << '\n';
    }
}

void test_compile_do() {
    std::cout << "Testing do block compilation..." << '\n';

    {
        LispParser parser("(do (+ 1 2) (* 3 4) (- 10 5))");
        auto ast = parser.parse();
        LispCompiler compiler;
        auto bytecode = compiler.compile(ast);

        StackVM vm;
        vm.load_program(bytecode);
        vm.execute();

        assert(vm.get_top() == 5);
        std::cout << "  ✓ Do block returns last value: 5" << '\n';
    }

    {
        LispParser parser("(do 42)");
        auto ast = parser.parse();
        LispCompiler compiler;
        auto bytecode = compiler.compile(ast);

        StackVM vm;
        vm.load_program(bytecode);
        vm.execute();

        assert(vm.get_top() == 42);
        std::cout << "  ✓ Single expression do: 42" << '\n';
    }
}

void test_compile_bitwise() {
    std::cout << "Testing bitwise operation compilation..." << '\n';

    {
        LispParser parser("(bit-and 12 10)");
        auto ast = parser.parse();
        LispCompiler compiler;
        auto bytecode = compiler.compile(ast);

        StackVM vm;
        vm.load_program(bytecode);
        vm.execute();

        assert(vm.get_top() == 8);
        std::cout << "  ✓ Bitwise AND: (bit-and 12 10) = 8" << '\n';
    }

    {
        LispParser parser("(bit-or 12 10)");
        auto ast = parser.parse();
        LispCompiler compiler;
        auto bytecode = compiler.compile(ast);

        StackVM vm;
        vm.load_program(bytecode);
        vm.execute();

        assert(vm.get_top() == 14);
        std::cout << "  ✓ Bitwise OR: (bit-or 12 10) = 14" << '\n';
    }

    {
        LispParser parser("(bit-xor 12 10)");
        auto ast = parser.parse();
        LispCompiler compiler;
        auto bytecode = compiler.compile(ast);

        StackVM vm;
        vm.load_program(bytecode);
        vm.execute();

        assert(vm.get_top() == 6);
        std::cout << "  ✓ Bitwise XOR: (bit-xor 12 10) = 6" << '\n';
    }

    {
        LispParser parser("(bit-shl 5 2)");
        auto ast = parser.parse();
        LispCompiler compiler;
        auto bytecode = compiler.compile(ast);

        StackVM vm;
        vm.load_program(bytecode);
        vm.execute();

        assert(vm.get_top() == 20);
        std::cout << "  ✓ Shift left: (bit-shl 5 2) = 20" << '\n';
    }

    {
        LispParser parser("(bit-shr 20 2)");
        auto ast = parser.parse();
        LispCompiler compiler;
        auto bytecode = compiler.compile(ast);

        StackVM vm;
        vm.load_program(bytecode);
        vm.execute();

        assert(vm.get_top() == 5);
        std::cout << "  ✓ Shift right: (bit-shr 20 2) = 5" << '\n';
    }
}

int main() {
    std::cout << "=== Compiler Basic Tests ===" << '\n';

    try {
        test_compile_number();
        std::cout << '\n';

        test_compile_arithmetic();
        std::cout << '\n';

        test_compile_comparison();
        std::cout << '\n';

        test_compile_nested();
        std::cout << '\n';

        test_compile_do();
        std::cout << '\n';

        test_compile_bitwise();

        std::cout << "\n✓ All compiler basic tests passed!" << '\n';
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Test failed: " << e.what() << '\n';
        return 1;
    }
}
