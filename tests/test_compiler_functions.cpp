#include "../src/lisp_compiler.hpp"
#include "../src/lisp_parser.hpp"
#include "../src/stack_vm.hpp"
#include <cassert>
#include <iostream>

void test_compile_simple_function() {
    std::cout << "Testing simple function..." << std::endl;

    std::string code = R"(
        (do
            (define-func (square x) (* x x))
            (square 7))
    )";

    LispParser parser(code);
    auto ast = parser.parse();
    LispCompiler compiler;
    auto bytecode = compiler.compile(ast);

    StackVM vm;
    vm.load_program(bytecode);
    vm.execute();

    assert(vm.get_top() == 49);
    std::cout << "  ✓ Simple function: (square 7) = 49" << std::endl;
}

void test_compile_function_two_params() {
    std::cout << "Testing function with two parameters..." << std::endl;

    std::string code = R"(
        (do
            (define-func (add a b) (+ a b))
            (add 10 20))
    )";

    LispParser parser(code);
    auto ast = parser.parse();
    LispCompiler compiler;
    auto bytecode = compiler.compile(ast);

    StackVM vm;
    vm.load_program(bytecode);
    vm.execute();

    assert(vm.get_top() == 30);
    std::cout << "  ✓ Two params: (add 10 20) = 30" << std::endl;
}

void test_compile_function_multiple_calls() {
    std::cout << "Testing multiple function calls..." << std::endl;

    std::string code = R"(
        (do
            (define-func (double x) (* x 2))
            (define-var a (double 5))
            (define-var b (double 10))
            (+ a b))
    )";

    LispParser parser(code);
    auto ast = parser.parse();
    LispCompiler compiler;
    auto bytecode = compiler.compile(ast);

    StackVM vm;
    vm.load_program(bytecode);
    vm.execute();

    // a = 10, b = 20, result = 30
    assert(vm.get_top() == 30);
    std::cout << "  ✓ Multiple calls: double(5) + double(10) = 30" << std::endl;
}

void test_compile_function_with_body() {
    std::cout << "Testing function with complex body..." << std::endl;

    std::string code = R"(
        (do
            (define-func (calculate x y)
                (do
                    (define-var sum (+ x y))
                    (define-var product (* x y))
                    (+ sum product)))
            (calculate 3 4))
    )";

    LispParser parser(code);
    auto ast = parser.parse();
    LispCompiler compiler;
    auto bytecode = compiler.compile(ast);

    StackVM vm;
    vm.load_program(bytecode);
    vm.execute();

    // sum = 7, product = 12, result = 19
    assert(vm.get_top() == 19);
    std::cout << "  ✓ Complex body: sum=7, product=12, result = 19" << std::endl;
}

void test_compile_recursive_function() {
    std::cout << "Testing recursive function (factorial)..." << std::endl;

    std::string code = R"(
        (do
            (define-func (factorial n)
                (if (= n 0)
                    1
                    (* n (factorial (- n 1)))))
            (factorial 5))
    )";

    LispParser parser(code);
    auto ast = parser.parse();
    LispCompiler compiler;
    auto bytecode = compiler.compile(ast);

    StackVM vm;
    vm.load_program(bytecode);
    vm.execute();

    assert(vm.get_top() == 120);
    std::cout << "  ✓ Recursive: factorial(5) = 120" << std::endl;
}

void test_compile_function_calling_function() {
    std::cout << "Testing function calling another function..." << std::endl;

    std::string code = R"(
        (do
            (define-func (double x) (* x 2))
            (define-func (quadruple x) (double (double x)))
            (quadruple 3))
    )";

    LispParser parser(code);
    auto ast = parser.parse();
    LispCompiler compiler;
    auto bytecode = compiler.compile(ast);

    StackVM vm;
    vm.load_program(bytecode);
    vm.execute();

    assert(vm.get_top() == 12);
    std::cout << "  ✓ Function calling function: quadruple(3) = 12" << std::endl;
}

void test_compile_function_with_conditionals() {
    std::cout << "Testing function with conditionals..." << std::endl;

    std::string code = R"(
        (do
            (define-func (max a b)
                (if (> a b) a b))
            (max 15 20))
    )";

    LispParser parser(code);
    auto ast = parser.parse();
    LispCompiler compiler;
    auto bytecode = compiler.compile(ast);

    StackVM vm;
    vm.load_program(bytecode);
    vm.execute();

    assert(vm.get_top() == 20);
    std::cout << "  ✓ Conditional function: max(15, 20) = 20" << std::endl;
}

void test_compile_fibonacci() {
    std::cout << "Testing fibonacci function..." << std::endl;

    std::string code = R"(
        (do
            (define-func (fib n)
                (if (< n 2)
                    n
                    (+ (fib (- n 1)) (fib (- n 2)))))
            (fib 5))
    )";

    LispParser parser(code);
    auto ast = parser.parse();
    LispCompiler compiler;
    auto bytecode = compiler.compile(ast);

    StackVM vm;
    vm.load_program(bytecode);
    vm.execute();

    // fib(5) = 5 (0,1,1,2,3,5)
    assert(vm.get_top() == 5);
    std::cout << "  ✓ Fibonacci: fib(5) = 5" << std::endl;
}

void test_compile_function_with_arguments_and_temporaries() {
    std::cout << "Testing function with arguments and temporaries..." << std::endl;

    std::string code = R"(
        (do
            (define-func (fn a b c)
		(let ((x (+ a 1)) (y (+ b 2)) (z (+ c 3)))
		     (+ (+ (+ a b) (+ c x)) (+ y z))))
            (fn 12 23 34))
    )";

    LispParser parser(code);
    auto ast = parser.parse();
    LispCompiler compiler;
    auto bytecode = compiler.compile(ast);

    StackVM vm;
    vm.load_program(bytecode);
    vm.execute();

    assert(vm.get_top() == 144);
    std::cout << "  ✓ fn: f(12, 23, 34) = 144" << std::endl;
}

int main() {
    std::cout << "=== Compiler Function Tests ===" << std::endl;

    try {
        test_compile_simple_function();
        test_compile_function_two_params();
        test_compile_function_multiple_calls();
        test_compile_function_with_body();
        std::cout << std::endl;

        test_compile_recursive_function();
        test_compile_fibonacci();
        test_compile_function_calling_function();
        test_compile_function_with_conditionals();
        test_compile_function_with_arguments_and_temporaries();

        std::cout << "\n✓ All compiler function tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Test failed: " << e.what() << std::endl;
        return 1;
    }
}
