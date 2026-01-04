#include "../src/lisp_compiler.hpp"
#include "../src/lisp_parser.hpp"
#include "../src/stack_vm.hpp"
#include <iostream>

void test(const std::string& name, const std::string& code) {
    std::cout << "=== " << name << " ===" << '\n';
    std::cout << "Code: " << code << '\n';

    try {
        LispParser parser(code);
        auto ast = parser.parse();

        LispCompiler compiler;
        auto bytecode = compiler.compile(ast);

        std::cout << "Bytecode size: " << bytecode.size() << " words" << '\n';

        StackVM vm;
        vm.load_program(bytecode);
        vm.execute();

        std::cout << "Result: " << vm.get_top() << '\n';
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
    }

    std::cout << '\n';
}

int main() {
    std::cout << "=== Testing Lambda Functions ===" << '\n' << '\n';

    // Simple functions
    std::cout << "--- Simple Functions ---" << '\n';
    test("Simple function", "(do "
                            "  (define-func (add-five x) (+ x 5)) "
                            "  (add-five 10))");

    test("Function with two parameters", "(do "
                                         "  (define-func (add x y) (+ x y)) "
                                         "  (add 15 27))");

    test("Function with multiple parameters", "(do "
                                              "  (define-func (sum3 a b c) (+ a b c)) "
                                              "  (sum3 10 20 30))");

    test("Zero parameter function", "(do "
                                    "  (define-func (get-forty-two) 42) "
                                    "  (get-forty-two))");

    // Arithmetic functions
    std::cout << "--- Arithmetic Functions ---" << '\n';
    test("Multiply function", "(do "
                              "  (define-func (mul x y) (* x y)) "
                              "  (mul 6 7))");

    test("Square function", "(do "
                            "  (define-func (square x) (* x x)) "
                            "  (square 9))");

    test("Multiple calls", "(do "
                           "  (define-func (double x) (* x 2)) "
                           "  (+ (double 5) (double 10)))");

    // Functions calling functions
    std::cout << "--- Function Composition ---" << '\n';
    test("Function calling another function", "(do "
                                              "  (define-func (add x y) (+ x y)) "
                                              "  (define-func (add-ten x) (add x 10)) "
                                              "  (add-ten 5))");

    test("Nested function calls", "(do "
                                  "  (define-func (inc x) (+ x 1)) "
                                  "  (define-func (double x) (* x 2)) "
                                  "  (double (inc 5)))");

    // Functions with expressions
    std::cout << "--- Complex Function Bodies ---" << '\n';
    test("Function with if", "(do "
                             "  (define-func (abs x) (if (< x 0) (- 0 x) x)) "
                             "  (abs -42))");

    test("Function with nested expressions", "(do "
                                             "  (define-func (formula x y) (+ (* x x) (* y y))) "
                                             "  (formula 3 4))");

    // Tagging functions
    std::cout << "--- Tagging Functions ---" << '\n';
    test("Tag integer function",
         "(do "
         "  (define-var TAG_INT 1) "
         "  (define-func (tag-int value) (bit-or (bit-shl value 3) TAG_INT)) "
         "  (tag-int 42))");

    test("Untag integer function", "(do "
                                   "  (define-func (untag-int tagged) (bit-ashr tagged 3)) "
                                   "  (untag-int 337))");

    test("Complete tagging system", "(do "
                                    "  (define-var TAG_INT 1) "
                                    "  (define-func (tag-int v) (bit-or (bit-shl v 3) TAG_INT)) "
                                    "  (define-func (untag-int t) (bit-ashr t 3)) "
                                    "  (define-func (is-int? t) (= (bit-and t 7) TAG_INT)) "
                                    "  (define-var original 99) "
                                    "  (define-var tagged (tag-int original)) "
                                    "  (print tagged) "
                                    "  (define-var untagged (untag-int tagged)) "
                                    "  (print untagged) "
                                    "  (define-var check (is-int? tagged)) "
                                    "  (print check) "
                                    "  untagged)");

    // Recursive-style (unrolled)
    std::cout << "--- Iterative Calculations ---" << '\n';
    test("Factorial unrolled", "(do "
                               "  (define-func (fact5) "
                               "    (do "
                               "      (define-var result 1) "
                               "      (set result (* result 5)) "
                               "      (set result (* result 4)) "
                               "      (set result (* result 3)) "
                               "      (set result (* result 2)) "
                               "      result)) "
                               "  (fact5))");

    test("Function with local variables", "(do "
                                          "  (define-func (compute x y) "
                                          "    (do "
                                          "      (define-var temp (* x 2)) "
                                          "      (define-var result (+ temp y)) "
                                          "      result)) "
                                          "  (compute 10 5))");

    std::cout << "All tests completed!" << '\n';

    return 0;
}
