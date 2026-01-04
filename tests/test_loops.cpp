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
    std::cout << "=== Testing Loop Constructs ===" << '\n' << '\n';

    // While loops
    std::cout << "--- WHILE Loops ---" << '\n';

    test("Simple while - count to 5", "(do "
                                      "  (define-var counter 0) "
                                      "  (while (< counter 5) "
                                      "    (do "
                                      "      (print counter) "
                                      "      (set counter (+ counter 1)))) "
                                      "  counter)");

    test("While with accumulator",
         "(do "
         "  (define-var sum 0) "
         "  (define-var i 1) "
         "  (while (< i 11) "
         "    (do "
         "      (set sum (+ sum i)) "
         "      (set i (+ i 1)))) "
         "  sum)"); // Sum of 1..10 = 55

    test("While computing factorial",
         "(do "
         "  (define-var n 5) "
         "  (define-var result 1) "
         "  (define-var i 1) "
         "  (while (< i (+ n 1)) "
         "    (do "
         "      (set result (* result i)) "
         "      (set i (+ i 1)))) "
         "  result)"); // 5! = 120

    test("While with early exit condition",
         "(do "
         "  (define-var x 0) "
         "  (while (< x 100) "
         "    (set x (+ x 7))) "
         "  x)"); // First x >= 100

    // For loops
    std::cout << "--- FOR Loops ---" << '\n';

    test("Simple for - print 0 to 4", "(do "
                                      "  (for (i 0 5) "
                                      "    (print i)) "
                                      "  99)");

    test("For with accumulator",
         "(do "
         "  (define-var sum 0) "
         "  (for (i 1 11) "
         "    (set sum (+ sum i))) "
         "  sum)"); // Sum of 1..10 = 55

    test("For computing factorial",
         "(do "
         "  (define-var result 1) "
         "  (for (i 1 6) "
         "    (set result (* result i))) "
         "  result)"); // 5! = 120

    test("For with multiplication table",
         "(do "
         "  (define-var product 0) "
         "  (for (i 1 6) "
         "    (set product (* i i))) "
         "  product)"); // Last iteration: 5*5 = 25

    test("Nested for loops",
         "(do "
         "  (define-var sum 0) "
         "  (for (i 1 4) "
         "    (for (j 1 4) "
         "      (set sum (+ sum 1)))) "
         "  sum)"); // 3 * 3 = 9 iterations

    // Combining loops with functions
    std::cout << "--- Loops with Functions ---" << '\n';

    test("Function using while",
         "(do "
         "  (define-func (factorial n) "
         "    (do "
         "      (define-var result 1) "
         "      (define-var i 1) "
         "      (while (< i (+ n 1)) "
         "        (do "
         "          (set result (* result i)) "
         "          (set i (+ i 1)))) "
         "      result)) "
         "  (factorial 6))"); // 6! = 720

    test("Function using for",
         "(do "
         "  (define-func (sum-range start end) "
         "    (do "
         "      (define-var total 0) "
         "      (for (i start end) "
         "        (set total (+ total i))) "
         "      total)) "
         "  (sum-range 1 11))"); // Sum of 1..10 = 55

    test("Function with nested loops",
         "(do "
         "  (define (multiply-sum a b) "
         "    (do "
         "      (define result 0) "
         "      (for (i 0 a) "
         "        (for (j 0 b) "
         "          (set result (+ result 1)))) "
         "      result)) "
         "  (multiply-sum 5 7))"); // 5 * 7 = 35

    // Loops with tagging
    std::cout << "--- Loops with Tagging ---" << '\n';

    test("Tag multiple integers", "(do "
                                  "  (define-var TAG_INT 1) "
                                  "  (define-func (tag-int v) (bit-or (bit-shl v 3) TAG_INT)) "
                                  "  (define-var last 0) "
                                  "  (for (i 0 5) "
                                  "    (do "
                                  "      (define-var tagged (tag-int i)) "
                                  "      (print tagged) "
                                  "      (set last tagged))) "
                                  "  last)");

    test("Build array of tagged integers",
         "(do "
         "  (define-var TAG_INT 1) "
         "  (define-func (tag-int v) (bit-or (bit-shl v 3) TAG_INT)) "
         "  (define-var sum 0) "
         "  (for (i 1 6) "
         "    (do "
         "      (define-var tagged (tag-int i)) "
         "      (define-func untagged (bit-ashr tagged 3)) "
         "      (set sum (+ sum untagged)))) "
         "  sum)"); // 1+2+3+4+5 = 15

    // Complex examples
    std::cout << "--- Complex Examples ---" << '\n';

    test("Fibonacci using while",
         "(do "
         "  (define-var a 0) "
         "  (define-var b 1) "
         "  (define-var n 10) "
         "  (define-var i 0) "
         "  (while (< i n) "
         "    (do "
         "      (define-var temp b) "
         "      (set b (+ a b)) "
         "      (set a temp) "
         "      (set i (+ i 1)))) "
         "  a)"); // 10th Fibonacci number = 55

    test("Power function using for",
         "(do "
         "  (define-func (power base exp) "
         "    (do "
         "      (define-var result 1) "
         "      (for (i 0 exp) "
         "        (set result (* result base))) "
         "      result)) "
         "  (power 2 10))"); // 2^10 = 1024

    test("GCD using while",
         "(do "
         "  (define-var a 48) "
         "  (define-var b 18) "
         "  (while (> b 0) "
         "    (do "
         "      (define-var temp b) "
         "      (set b (% a b)) "
         "      (set a temp))) "
         "  a)"); // GCD(48, 18) = 6

    std::cout << "All tests completed!" << '\n';

    return 0;
}
