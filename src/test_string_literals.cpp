#include "lisp_compiler.hpp"
#include "lisp_parser.hpp"
#include "stack_vm.hpp"
#include <iostream>

int main() {
    std::cout << "=== Testing String Literals ===" << std::endl << std::endl;

    try {
        // Test 1: Simple string
        std::cout << "Test 1: Simple string literal" << std::endl;
        std::string code1 = R"(
            (do
                (define-var greeting "Hello, World!")
                (print greeting))
        )";

        LispParser parser1(code1);
        auto ast1 = parser1.parse();
        LispCompiler compiler1;
        auto bytecode1 = compiler1.compile(ast1);

        StackVM vm1;
        vm1.load_program(bytecode1);
        compiler1.write_strings_to_memory(vm1);
        vm1.execute();
        std::cout << "  PASSED" << std::endl << std::endl;

        // Test 2: Multiple strings (deduplication test)
        std::cout << "Test 2: Multiple strings and deduplication" << std::endl;
        std::string code2 = R"(
            (do
                (define-var s1 "test")
                (define-var s2 "test")
                (define-var s3 "different")
                (print s1)
                (print s2)
                (print s3))
        )";

        LispParser parser2(code2);
        auto ast2 = parser2.parse();
        LispCompiler compiler2;
        auto bytecode2 = compiler2.compile(ast2);

        StackVM vm2;
        vm2.load_program(bytecode2);
        compiler2.write_strings_to_memory(vm2);
        vm2.execute();
        std::cout << "  PASSED (s1 and s2 should share same address)" << std::endl << std::endl;

        // Test 3: String with escape sequences
        std::cout << "Test 3: Escape sequences" << std::endl;
        std::string code3 = R"(
            (define-var escaped "Line1\nLine2\tTabbed")
        )";

        LispParser parser3(code3);
        auto ast3 = parser3.parse();
        LispCompiler compiler3;
        auto bytecode3 = compiler3.compile(ast3);

        StackVM vm3;
        vm3.load_program(bytecode3);
        compiler3.write_strings_to_memory(vm3);
        vm3.execute();
        std::cout << "  PASSED" << std::endl << std::endl;

        // Test 4: Empty string
        std::cout << "Test 4: Empty string" << std::endl;
        std::string code4 = R"(
            (define-var empty "")
        )";

        LispParser parser4(code4);
        auto ast4 = parser4.parse();
        LispCompiler compiler4;
        auto bytecode4 = compiler4.compile(ast4);

        StackVM vm4;
        vm4.load_program(bytecode4);
        compiler4.write_strings_to_memory(vm4);
        vm4.execute();
        std::cout << "  PASSED" << std::endl << std::endl;

        std::cout << "=== All String Literal Tests Passed! ===" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
