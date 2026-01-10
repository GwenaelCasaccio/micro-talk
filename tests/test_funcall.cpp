#include "../src/lisp_compiler.hpp"
#include "../src/lisp_parser.hpp"
#include "../src/stack_vm.hpp"
#include <cassert>
#include <iostream>

int main() {
    std::cout << "=== Testing funcall primitive ===" << std::endl;
    std::cout << std::endl;

    try {
        // Test 1: Simple funcall with no arguments
        std::cout << "Test 1: funcall with no arguments" << std::endl;
        std::string code1 = R"(
            (do
                (define-func (return-42)
                    42)

                (funcall (function-address return-42)))
        )";

        LispParser parser1(code1);
        auto ast1 = parser1.parse();
        LispCompiler compiler1;
        auto program1 = compiler1.compile(ast1);

        StackVM vm1;
        vm1.load_program(program1);
        vm1.execute();

        uint64_t result1 = vm1.get_top();
        assert(result1 == 42);
        std::cout << "  ✓ Result: " << result1 << " (expected 42)" << std::endl;
        std::cout << std::endl;

        // Test 2: funcall with one argument
        std::cout << "Test 2: funcall with one argument" << std::endl;
        std::string code2 = R"(
            (do
                (define-func (double x)
                    (* x 2))

                (funcall (function-address double) 21))
        )";

        LispParser parser2(code2);
        auto ast2 = parser2.parse();
        LispCompiler compiler2;
        auto program2 = compiler2.compile(ast2);

        StackVM vm2;
        vm2.load_program(program2);
        vm2.execute();

        uint64_t result2 = vm2.get_top();
        assert(result2 == 42);
        std::cout << "  ✓ Result: " << result2 << " (expected 42)" << std::endl;
        std::cout << std::endl;

        // Test 3: funcall with multiple arguments
        std::cout << "Test 3: funcall with multiple arguments" << std::endl;
        std::string code3 = R"(
            (do
                (define-func (add-three a b c)
                    (+ a b c))

                (funcall (function-address add-three) 10 20 12))
        )";

        LispParser parser3(code3);
        auto ast3 = parser3.parse();
        LispCompiler compiler3;
        auto program3 = compiler3.compile(ast3);

        StackVM vm3;
        vm3.load_program(program3);
        vm3.execute();

        uint64_t result3 = vm3.get_top();
        assert(result3 == 42);
        std::cout << "  ✓ Result: " << result3 << " (expected 42)" << std::endl;
        std::cout << std::endl;

        // Test 4: funcall with dynamic address
        std::cout << "Test 4: funcall with dynamic address from variable" << std::endl;
        std::string code4 = R"(
            (do
                (define-func (square x)
                    (* x x))

                (define-var func-addr (function-address square))
                (funcall func-addr 7))
        )";

        LispParser parser4(code4);
        auto ast4 = parser4.parse();
        LispCompiler compiler4;
        auto program4 = compiler4.compile(ast4);

        StackVM vm4;
        vm4.load_program(program4);
        vm4.execute();

        uint64_t result4 = vm4.get_top();
        assert(result4 == 49);
        std::cout << "  ✓ Result: " << result4 << " (expected 49)" << std::endl;
        std::cout << std::endl;

        // Test 5: Nested funcalls
        std::cout << "Test 5: nested funcalls" << std::endl;
        std::string code5 = R"(
            (do
                (define-func (add a b)
                    (+ a b))

                (define-func (multiply a b)
                    (* a b))

                (funcall (function-address add)
                    (funcall (function-address multiply) 3 10)
                    (funcall (function-address multiply) 2 6)))
        )";

        LispParser parser5(code5);
        auto ast5 = parser5.parse();
        LispCompiler compiler5;
        auto program5 = compiler5.compile(ast5);

        StackVM vm5;
        vm5.load_program(program5);
        vm5.execute();

        uint64_t result5 = vm5.get_top();
        assert(result5 == 42); // (3*10) + (2*6) = 30 + 12 = 42
        std::cout << "  ✓ Result: " << result5 << " (expected 42)" << std::endl;
        std::cout << std::endl;

        std::cout << "=== All funcall tests passed! ===" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
