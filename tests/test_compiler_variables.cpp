#include "../src/lisp_compiler.hpp"
#include "../src/lisp_parser.hpp"
#include "../src/stack_vm.hpp"
#include <cassert>
#include <iostream>

void test_compile_define() {
    std::cout << "Testing variable definition..." << '\n';

    std::string code = "(do (define-var x 42) x)";
    LispParser parser(code);
    auto ast = parser.parse();
    LispCompiler compiler;
    auto bytecode = compiler.compile(ast);

    StackVM vm;
    vm.load_program(bytecode);
    vm.execute();

    assert(vm.get_top() == 42);
    std::cout << "  ✓ Define and use: (define-var x 42) x = 42" << '\n';
}

void test_compile_multiple_defines() {
    std::cout << "Testing multiple definitions..." << '\n';

    std::string code = R"(
        (do
            (define-var x 10)
            (define-var y 20)
            (define-var z 30)
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
    std::cout << "  ✓ Multiple defines: x=10, y=20, z=30, x+y+z = 60" << '\n';
}

void test_compile_set() {
    std::cout << "Testing variable assignment..." << '\n';

    std::string code = R"(
        (do
            (define-var x 10)
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
    std::cout << "  ✓ Set variable: x=10, set x=42, x = 42" << '\n';
}

void test_compile_set_with_expression() {
    std::cout << "Testing set with expression..." << '\n';

    std::string code = R"(
        (do
            (define-var x 10)
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
    std::cout << "  ✓ Set with expr: x=10, x=x+5, x = 15" << '\n';
}

void test_compile_let() {
    std::cout << "Testing let binding..." << '\n';

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
    std::cout << "  ✓ Let binding: (let ((x 10) (y 20)) (+ x y)) = 30" << '\n';
}

void test_compile_let_shadowing() {
    std::cout << "Testing let variable shadowing..." << '\n';

    std::string code = R"(
        (do
            (define-var x 100)
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
    std::cout << "  ✓ Let shadowing: outer x=100, let x=42 returns 42" << '\n';
}

void test_compile_let_multiple_body() {
    std::cout << "Testing let with multiple body expressions..." << '\n';

    std::string code = R"(
        (let ((x 10) (y 5))
            (define-var z (* x y))
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
    std::cout << "  ✓ Let multiple body: z=x*y, z+10 = 60" << '\n';
}

void test_compile_nested_let() {
    std::cout << "Testing nested let..." << '\n';

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
    std::cout << "  ✓ Nested let: outer x=10, inner y=20, x+y = 30" << '\n';
}

void test_compile_variable_arithmetic() {
    std::cout << "Testing variable arithmetic..." << '\n';

    std::string code = R"(
        (do
            (define-var a 5)
            (define-var b 3)
            (define-var sum (+ a b))
            (define-var product (* a b))
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
    std::cout << "  ✓ Variable arithmetic: sum=8, product=15, sum+product = 23" << '\n';
}

int main() {
    std::cout << "=== Compiler Variable Tests ===" << '\n';

    try {
        test_compile_define();
        test_compile_multiple_defines();
        test_compile_set();
        test_compile_set_with_expression();
        std::cout << '\n';

        test_compile_let();
        test_compile_let_shadowing();
        test_compile_let_multiple_body();
        test_compile_nested_let();
        std::cout << '\n';

        test_compile_variable_arithmetic();

        std::cout << "\n✓ All compiler variable tests passed!" << '\n';
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Test failed: " << e.what() << '\n';
        return 1;
    }
}
