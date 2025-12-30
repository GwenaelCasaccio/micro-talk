#include "../src/stack_vm.hpp"
#include "../src/lisp_parser.hpp"
#include "../src/lisp_compiler.hpp"
#include <iostream>
#include <cassert>

void test_compile_number() {
    std::cout << "Testing number compilation..." << std::endl;

    LispParser parser("42");
    auto ast = parser.parse();
    LispCompiler compiler;
    auto bytecode = compiler.compile(ast);

    StackVM vm;
    vm.load_program(bytecode);
    vm.execute();

    assert(vm.get_top() == 42);
    std::cout << "  ✓ Number literal: 42" << std::endl;
}

void test_compile_arithmetic() {
    std::cout << "Testing arithmetic compilation..." << std::endl;

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
        std::cout << "  ✓ Addition: (+ 10 20) = 30" << std::endl;
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
        std::cout << "  ✓ Subtraction: (- 50 20) = 30" << std::endl;
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
        std::cout << "  ✓ Multiplication: (* 6 7) = 42" << std::endl;
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
        std::cout << "  ✓ Division: (/ 100 5) = 20" << std::endl;
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
        std::cout << "  ✓ Modulo: (% 17 5) = 2" << std::endl;
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
        std::cout << "  ✓ Multi-arg addition: (+ 1 2 3 4 5) = 15" << std::endl;
    }
}

void test_compile_comparison() {
    std::cout << "Testing comparison compilation..." << std::endl;

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
        std::cout << "  ✓ Equal (true): (= 42 42) = 1" << std::endl;
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
        std::cout << "  ✓ Equal (false): (= 42 100) = 0" << std::endl;
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
        std::cout << "  ✓ Less than (true): (< 10 20) = 1" << std::endl;
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
        std::cout << "  ✓ Greater than (false): (> 5 10) = 0" << std::endl;
    }
}

void test_compile_nested() {
    std::cout << "Testing nested expression compilation..." << std::endl;

    {
        LispParser parser("(+ (* 2 3) 4)");
        auto ast = parser.parse();
        LispCompiler compiler;
        auto bytecode = compiler.compile(ast);

        StackVM vm;
        vm.load_program(bytecode);
        vm.execute();

        assert(vm.get_top() == 10);
        std::cout << "  ✓ Nested: (+ (* 2 3) 4) = 10" << std::endl;
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
        std::cout << "  ✓ Complex nested: (* (+ 2 3) (- 10 4)) = 30" << std::endl;
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
        std::cout << "  ✓ Deep nested: (+ (* 2 (+ 3 4)) (- 10 5)) = 19" << std::endl;
    }
}

void test_compile_do() {
    std::cout << "Testing do block compilation..." << std::endl;

    {
        LispParser parser("(do (+ 1 2) (* 3 4) (- 10 5))");
        auto ast = parser.parse();
        LispCompiler compiler;
        auto bytecode = compiler.compile(ast);

        StackVM vm;
        vm.load_program(bytecode);
        vm.execute();

        assert(vm.get_top() == 5);
        std::cout << "  ✓ Do block returns last value: 5" << std::endl;
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
        std::cout << "  ✓ Single expression do: 42" << std::endl;
    }
}

void test_compile_bitwise() {
    std::cout << "Testing bitwise operation compilation..." << std::endl;

    {
        LispParser parser("(bit-and 12 10)");
        auto ast = parser.parse();
        LispCompiler compiler;
        auto bytecode = compiler.compile(ast);

        StackVM vm;
        vm.load_program(bytecode);
        vm.execute();

        assert(vm.get_top() == 8);
        std::cout << "  ✓ Bitwise AND: (bit-and 12 10) = 8" << std::endl;
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
        std::cout << "  ✓ Bitwise OR: (bit-or 12 10) = 14" << std::endl;
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
        std::cout << "  ✓ Bitwise XOR: (bit-xor 12 10) = 6" << std::endl;
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
        std::cout << "  ✓ Shift left: (bit-shl 5 2) = 20" << std::endl;
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
        std::cout << "  ✓ Shift right: (bit-shr 20 2) = 5" << std::endl;
    }
}

int main() {
    std::cout << "=== Compiler Basic Tests ===" << std::endl;

    try {
        test_compile_number();
        std::cout << std::endl;

        test_compile_arithmetic();
        std::cout << std::endl;

        test_compile_comparison();
        std::cout << std::endl;

        test_compile_nested();
        std::cout << std::endl;

        test_compile_do();
        std::cout << std::endl;

        test_compile_bitwise();

        std::cout << "\n✓ All compiler basic tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Test failed: " << e.what() << std::endl;
        return 1;
    }
}
