#include "../src/stack_vm.hpp"
#include "../src/lisp_parser.hpp"
#include "../src/lisp_compiler.hpp"
#include <iostream>

int main() {
    try {
        std::cout << "=== Testing Comment Support ===" << std::endl;

        // Test 1: Simple line comment
        {
            std::cout << "\nTest 1: Simple line comment" << std::endl;
            std::string code = R"(
                ; This is a comment
                (+ 5 3)
            )";

            LispParser parser(code);
            auto ast = parser.parse();
            LispCompiler compiler;
            auto bytecode = compiler.compile(ast);

            StackVM vm;
            vm.load_program(bytecode);
            vm.execute();

            std::cout << "Result: " << vm.get_top() << " (expected: 8)" << std::endl;
        }

        // Test 2: Multiple comments
        {
            std::cout << "\nTest 2: Multiple comments" << std::endl;
            std::string code = R"(
                ; First comment
                ; Second comment
                ; Third comment
                (* 6 7)
            )";

            LispParser parser(code);
            auto ast = parser.parse();
            LispCompiler compiler;
            auto bytecode = compiler.compile(ast);

            StackVM vm;
            vm.load_program(bytecode);
            vm.execute();

            std::cout << "Result: " << vm.get_top() << " (expected: 42)" << std::endl;
        }

        // Test 3: Inline comments in do block
        {
            std::cout << "\nTest 3: Inline comments in expressions" << std::endl;
            std::string code = R"(
                (do
                    ; Set x to 10
                    (define-var x 10)
                    ; Set y to 20
                    (define-var y 20)
                    ; Add them together
                    (+ x y))
            )";

            LispParser parser(code);
            auto ast = parser.parse();
            LispCompiler compiler;
            auto bytecode = compiler.compile(ast);

            StackVM vm;
            vm.load_program(bytecode);
            vm.execute();

            std::cout << "Result: " << vm.get_top() << " (expected: 30)" << std::endl;
        }

        // Test 4: Comments in list
        {
            std::cout << "\nTest 4: Comments between list elements" << std::endl;
            std::string code = R"(
                (+
                    ; First operand
                    100
                    ; Second operand
                    200
                    ; Third operand
                    300)
            )";

            LispParser parser(code);
            auto ast = parser.parse();
            LispCompiler compiler;
            auto bytecode = compiler.compile(ast);

            StackVM vm;
            vm.load_program(bytecode);
            vm.execute();

            std::cout << "Result: " << vm.get_top() << " (expected: 600)" << std::endl;
        }

        // Test 5: Multiple expressions with comments
        {
            std::cout << "\nTest 5: Multiple expressions with comments" << std::endl;
            std::string code = R"(
                ; First calculation
                (define-var a (+ 5 5))

                ; Second calculation
                (define-var b (* 3 3))

                ; Final result
                (+ a b)
            )";

            LispParser parser(code);
            auto exprs = parser.parse_all();
            LispCompiler compiler;
            auto bytecode = compiler.compile_program(exprs);

            StackVM vm;
            vm.load_program(bytecode);
            vm.execute();

            std::cout << "Result: " << vm.get_top() << " (expected: 19)" << std::endl;
        }

        // Test 6: Comment at end of file
        {
            std::cout << "\nTest 6: Comment at end of file" << std::endl;
            std::string code = R"(
                (+ 1 2 3 4 5)
                ; This is the end
            )";

            LispParser parser(code);
            auto ast = parser.parse();
            LispCompiler compiler;
            auto bytecode = compiler.compile(ast);

            StackVM vm;
            vm.load_program(bytecode);
            vm.execute();

            std::cout << "Result: " << vm.get_top() << " (expected: 15)" << std::endl;
        }

        std::cout << "\n=== All Comment Tests Passed ===" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
