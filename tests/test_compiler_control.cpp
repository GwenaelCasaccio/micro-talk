#include "../src/lisp_compiler.hpp"
#include "../src/lisp_parser.hpp"
#include "../src/stack_vm.hpp"
#include <cassert>
#include <iostream>

void test_compile_if_then() {
    std::cout << "Testing if (then branch)..." << std::endl;

    LispParser parser("(if (< 5 10) 100 200)");
    auto ast = parser.parse();
    LispCompiler compiler;
    auto bytecode = compiler.compile(ast);

    StackVM vm;
    vm.load_program(bytecode);
    vm.execute();

    assert(vm.get_top() == 100);
    std::cout << "  ✓ If (true condition): (if (< 5 10) 100 200) = 100" << std::endl;
}

void test_compile_if_else() {
    std::cout << "Testing if (else branch)..." << std::endl;

    LispParser parser("(if (> 5 10) 100 200)");
    auto ast = parser.parse();
    LispCompiler compiler;
    auto bytecode = compiler.compile(ast);

    StackVM vm;
    vm.load_program(bytecode);
    vm.execute();

    assert(vm.get_top() == 200);
    std::cout << "  ✓ If (false condition): (if (> 5 10) 100 200) = 200" << std::endl;
}

void test_compile_nested_if() {
    std::cout << "Testing nested if..." << std::endl;

    LispParser parser("(if (< 5 10) (if (= 3 3) 42 0) 99)");
    auto ast = parser.parse();
    LispCompiler compiler;
    auto bytecode = compiler.compile(ast);

    StackVM vm;
    vm.load_program(bytecode);
    vm.execute();

    assert(vm.get_top() == 42);
    std::cout << "  ✓ Nested if: (if (< 5 10) (if (= 3 3) 42 0) 99) = 42" << std::endl;
}

void test_compile_if_with_expressions() {
    std::cout << "Testing if with complex expressions..." << std::endl;

    LispParser parser("(if (= (% 10 3) 1) (* 5 5) (+ 1 1))");
    auto ast = parser.parse();
    LispCompiler compiler;
    auto bytecode = compiler.compile(ast);

    StackVM vm;
    vm.load_program(bytecode);
    vm.execute();

    assert(vm.get_top() == 25);
    std::cout << "  ✓ If with expressions: (if (= (% 10 3) 1) (* 5 5) (+ 1 1)) = 25" << std::endl;
}

void test_compile_while() {
    std::cout << "Testing while loop..." << std::endl;

    std::string code = R"(
        (do
            (define-var counter 0)
            (define-var sum 0)
            (while (< counter 5)
                (do
                    (set sum (+ sum counter))
                    (set counter (+ counter 1))))
            sum)
    )";

    LispParser parser(code);
    auto ast = parser.parse();
    LispCompiler compiler;
    auto bytecode = compiler.compile(ast);

    StackVM vm;
    vm.load_program(bytecode);
    vm.execute();

    // sum = 0 + 1 + 2 + 3 + 4 = 10
    assert(vm.get_top() == 10);
    std::cout << "  ✓ While loop: sum 0+1+2+3+4 = 10" << std::endl;
}

void test_compile_for() {
    std::cout << "Testing for loop..." << std::endl;

    std::string code = R"(
        (do
            (define-var total 0)
            (for (i 0 5)
                (set total (+ total i)))
            total)
    )";

    LispParser parser(code);
    auto ast = parser.parse();
    LispCompiler compiler;
    auto bytecode = compiler.compile(ast);

    StackVM vm;
    vm.load_program(bytecode);
    vm.execute();

    // total = 0 + 1 + 2 + 3 + 4 = 10
    assert(vm.get_top() == 10);
    std::cout << "  ✓ For loop: sum 0+1+2+3+4 = 10" << std::endl;
}

void test_compile_for_nested() {
    std::cout << "Testing nested for loops..." << std::endl;

    std::string code = R"(
        (do
            (define-var result 0)
            (for (i 1 4)
                (for (j 1 4)
                    (set result (+ result 1))))
            result)
    )";

    LispParser parser(code);
    auto ast = parser.parse();
    LispCompiler compiler;
    auto bytecode = compiler.compile(ast);

    StackVM vm;
    vm.load_program(bytecode);
    vm.execute();

    // 3 iterations * 3 iterations = 9
    assert(vm.get_top() == 9);
    std::cout << "  ✓ Nested for: 3x3 iterations = 9" << std::endl;
}

void test_compile_while_with_condition() {
    std::cout << "Testing while with complex condition..." << std::endl;

    std::string code = R"(
        (do
            (define-var x 10)
            (while (> x 0)
                (set x (- x 2)))
            x)
    )";

    LispParser parser(code);
    auto ast = parser.parse();
    LispCompiler compiler;
    auto bytecode = compiler.compile(ast);

    StackVM vm;
    vm.load_program(bytecode);
    vm.execute();

    // 10, 8, 6, 4, 2, 0
    assert(vm.get_top() == 0);
    std::cout << "  ✓ While countdown: 10->8->6->4->2->0" << std::endl;
}

int main() {
    std::cout << "=== Compiler Control Flow Tests ===" << std::endl;

    try {
        test_compile_if_then();
        test_compile_if_else();
        test_compile_nested_if();
        test_compile_if_with_expressions();
        std::cout << std::endl;

        test_compile_while();
        test_compile_for();
        test_compile_for_nested();
        test_compile_while_with_condition();

        std::cout << "\n✓ All compiler control flow tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Test failed: " << e.what() << std::endl;
        return 1;
    }
}
