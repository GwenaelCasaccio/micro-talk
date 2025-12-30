#include "../src/stack_vm.hpp"
#include "../src/lisp_parser.hpp"
#include "../src/lisp_compiler.hpp"
#include <iostream>
#include <cassert>

void test_compile_define() {
    std::cout << "Testing variable definition..." << std::endl;

    std::string code = "(do (define x 42) x)";
    LispParser parser(code);
    auto ast = parser.parse();
    LispCompiler compiler;
    auto bytecode = compiler.compile(ast);

    StackVM vm;
    vm.load_program(bytecode);
    vm.execute();

    assert(vm.get_top() == 42);
    std::cout << "  ✓ Define and use: (define x 42) x = 42" << std::endl;
}

void test_compile_multiple_defines() {
    std::cout << "Testing multiple definitions..." << std::endl;

    std::string code = R"(
        (do
            (define x 10)
            (define y 20)
            (define z 30)
            (+ x (+ y z)))
    )";

    LispParser parser(code);
    auto ast = parser.parse();
    LispCompiler compiler;
    auto bytecode = compiler.compile(ast);

    StackVM vm;
    vm.load_program(bytecode);
    vm.execute();

    assert(vm.get_top() == 60);
    std::cout << "  ✓ Multiple defines: x=10, y=20, z=30, x+y+z = 60" << std::endl;
}

void test_compile_set() {
    std::cout << "Testing variable assignment..." << std::endl;

    std::string code = R"(
        (do
            (define x 10)
            (set x 42)
            x)
    )";

    LispParser parser(code);
    auto ast = parser.parse();
    LispCompiler compiler;
    auto bytecode = compiler.compile(ast);

    StackVM vm;
    vm.load_program(bytecode);
    vm.execute();

    assert(vm.get_top() == 42);
    std::cout << "  ✓ Set variable: x=10, set x=42, x = 42" << std::endl;
}

void test_compile_set_with_expression() {
    std::cout << "Testing set with expression..." << std::endl;

    std::string code = R"(
        (do
            (define x 10)
            (set x (+ x 5))
            x)
    )";

    LispParser parser(code);
    auto ast = parser.parse();
    LispCompiler compiler;
    auto bytecode = compiler.compile(ast);

    StackVM vm;
    vm.load_program(bytecode);
    vm.execute();

    assert(vm.get_top() == 15);
    std::cout << "  ✓ Set with expr: x=10, x=x+5, x = 15" << std::endl;
}

void test_compile_let() {
    std::cout << "Testing let binding..." << std::endl;

    std::string code = R"(
        (let ((x 10) (y 20))
            (+ x y))
    )";

    LispParser parser(code);
    auto ast = parser.parse();
    LispCompiler compiler;
    auto bytecode = compiler.compile(ast);

    StackVM vm;
    vm.load_program(bytecode);
    vm.execute();

    assert(vm.get_top() == 30);
    std::cout << "  ✓ Let binding: (let ((x 10) (y 20)) (+ x y)) = 30" << std::endl;
}

void test_compile_let_shadowing() {
    std::cout << "Testing let variable shadowing..." << std::endl;

    std::string code = R"(
        (do
            (define x 100)
            (let ((x 42))
                x))
    )";

    LispParser parser(code);
    auto ast = parser.parse();
    LispCompiler compiler;
    auto bytecode = compiler.compile(ast);

    StackVM vm;
    vm.load_program(bytecode);
    vm.execute();

    assert(vm.get_top() == 42);
    std::cout << "  ✓ Let shadowing: outer x=100, let x=42 returns 42" << std::endl;
}

void test_compile_let_multiple_body() {
    std::cout << "Testing let with multiple body expressions..." << std::endl;

    std::string code = R"(
        (let ((x 10) (y 5))
            (define z (* x y))
            (+ z 10))
    )";

    LispParser parser(code);
    auto ast = parser.parse();
    LispCompiler compiler;
    auto bytecode = compiler.compile(ast);

    StackVM vm;
    vm.load_program(bytecode);
    vm.execute();

    // z = 10 * 5 = 50, result = 50 + 10 = 60
    assert(vm.get_top() == 60);
    std::cout << "  ✓ Let multiple body: z=x*y, z+10 = 60" << std::endl;
}

void test_compile_nested_let() {
    std::cout << "Testing nested let..." << std::endl;

    std::string code = R"(
        (let ((x 10))
            (let ((y 20))
                (+ x y)))
    )";

    LispParser parser(code);
    auto ast = parser.parse();
    LispCompiler compiler;
    auto bytecode = compiler.compile(ast);

    StackVM vm;
    vm.load_program(bytecode);
    vm.execute();

    assert(vm.get_top() == 30);
    std::cout << "  ✓ Nested let: outer x=10, inner y=20, x+y = 30" << std::endl;
}

void test_compile_variable_arithmetic() {
    std::cout << "Testing variable arithmetic..." << std::endl;

    std::string code = R"(
        (do
            (define a 5)
            (define b 3)
            (define sum (+ a b))
            (define product (* a b))
            (+ sum product))
    )";

    LispParser parser(code);
    auto ast = parser.parse();
    LispCompiler compiler;
    auto bytecode = compiler.compile(ast);

    StackVM vm;
    vm.load_program(bytecode);
    vm.execute();

    // sum = 8, product = 15, result = 23
    assert(vm.get_top() == 23);
    std::cout << "  ✓ Variable arithmetic: sum=8, product=15, sum+product = 23" << std::endl;
}

int main() {
    std::cout << "=== Compiler Variable Tests ===" << std::endl;

    try {
        test_compile_define();
        test_compile_multiple_defines();
        test_compile_set();
        test_compile_set_with_expression();
        std::cout << std::endl;

        test_compile_let();
        test_compile_let_shadowing();
        test_compile_let_multiple_body();
        test_compile_nested_let();
        std::cout << std::endl;

        test_compile_variable_arithmetic();

        std::cout << "\n✓ All compiler variable tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Test failed: " << e.what() << std::endl;
        return 1;
    }
}
