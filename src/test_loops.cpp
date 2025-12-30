#include "stack_vm.hpp"
#include "lisp_parser.hpp"
#include "lisp_compiler.hpp"
#include <iostream>

void test(const std::string& name, const std::string& code) {
    std::cout << "=== " << name << " ===" << std::endl;
    std::cout << "Code: " << code << std::endl;
    
    try {
        LispParser parser(code);
        auto ast = parser.parse();
        
        LispCompiler compiler;
        auto bytecode = compiler.compile(ast);
        
        StackVM vm;
        vm.load_program(bytecode);
        vm.execute();
        
        std::cout << "Result: " << vm.get_top() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    std::cout << std::endl;
}

int main() {
    std::cout << "=== Testing Loop Constructs ===" << std::endl << std::endl;
    
    // While loops
    std::cout << "--- WHILE Loops ---" << std::endl;
    
    test("Simple while - count to 5",
         "(do "
         "  (define counter 0) "
         "  (while (< counter 5) "
         "    (do "
         "      (print counter) "
         "      (set counter (+ counter 1)))) "
         "  counter)");
    
    test("While with accumulator",
         "(do "
         "  (define sum 0) "
         "  (define i 1) "
         "  (while (< i 11) "
         "    (do "
         "      (set sum (+ sum i)) "
         "      (set i (+ i 1)))) "
         "  sum)");  // Sum of 1..10 = 55
    
    test("While computing factorial",
         "(do "
         "  (define n 5) "
         "  (define result 1) "
         "  (define i 1) "
         "  (while (< i (+ n 1)) "
         "    (do "
         "      (set result (* result i)) "
         "      (set i (+ i 1)))) "
         "  result)");  // 5! = 120
    
    test("While with early exit condition",
         "(do "
         "  (define x 0) "
         "  (while (< x 100) "
         "    (set x (+ x 7))) "
         "  x)");  // First x >= 100
    
    // For loops
    std::cout << "--- FOR Loops ---" << std::endl;
    
    test("Simple for - print 0 to 4",
         "(do "
         "  (for (i 0 5) "
         "    (print i)) "
         "  99)");
    
    test("For with accumulator",
         "(do "
         "  (define sum 0) "
         "  (for (i 1 11) "
         "    (set sum (+ sum i))) "
         "  sum)");  // Sum of 1..10 = 55
    
    test("For computing factorial",
         "(do "
         "  (define result 1) "
         "  (for (i 1 6) "
         "    (set result (* result i))) "
         "  result)");  // 5! = 120
    
    test("For with multiplication table",
         "(do "
         "  (define product 0) "
         "  (for (i 1 6) "
         "    (set product (* i i))) "
         "  product)");  // Last iteration: 5*5 = 25
    
    test("Nested for loops",
         "(do "
         "  (define sum 0) "
         "  (for (i 1 4) "
         "    (for (j 1 4) "
         "      (set sum (+ sum 1)))) "
         "  sum)");  // 3 * 3 = 9 iterations
    
    // Combining loops with functions
    std::cout << "--- Loops with Functions ---" << std::endl;
    
    test("Function using while",
         "(do "
         "  (define (factorial n) "
         "    (do "
         "      (define result 1) "
         "      (define i 1) "
         "      (while (< i (+ n 1)) "
         "        (do "
         "          (set result (* result i)) "
         "          (set i (+ i 1)))) "
         "      result)) "
         "  (factorial 6))");  // 6! = 720
    
    test("Function using for",
         "(do "
         "  (define (sum-range start end) "
         "    (do "
         "      (define total 0) "
         "      (for (i start end) "
         "        (set total (+ total i))) "
         "      total)) "
         "  (sum-range 1 11))");  // Sum of 1..10 = 55
    
    test("Function with nested loops",
         "(do "
         "  (define (multiply-sum a b) "
         "    (do "
         "      (define result 0) "
         "      (for (i 0 a) "
         "        (for (j 0 b) "
         "          (set result (+ result 1)))) "
         "      result)) "
         "  (multiply-sum 5 7))");  // 5 * 7 = 35
    
    // Loops with tagging
    std::cout << "--- Loops with Tagging ---" << std::endl;
    
    test("Tag multiple integers",
         "(do "
         "  (define TAG_INT 1) "
         "  (define (tag-int v) (bit-or (bit-shl v 3) TAG_INT)) "
         "  (define last 0) "
         "  (for (i 0 5) "
         "    (do "
         "      (define tagged (tag-int i)) "
         "      (print tagged) "
         "      (set last tagged))) "
         "  last)");
    
    test("Build array of tagged integers",
         "(do "
         "  (define TAG_INT 1) "
         "  (define (tag-int v) (bit-or (bit-shl v 3) TAG_INT)) "
         "  (define sum 0) "
         "  (for (i 1 6) "
         "    (do "
         "      (define tagged (tag-int i)) "
         "      (define untagged (bit-ashr tagged 3)) "
         "      (set sum (+ sum untagged)))) "
         "  sum)");  // 1+2+3+4+5 = 15
    
    // Complex examples
    std::cout << "--- Complex Examples ---" << std::endl;
    
    test("Fibonacci using while",
         "(do "
         "  (define a 0) "
         "  (define b 1) "
         "  (define n 10) "
         "  (define i 0) "
         "  (while (< i n) "
         "    (do "
         "      (define temp b) "
         "      (set b (+ a b)) "
         "      (set a temp) "
         "      (set i (+ i 1)))) "
         "  a)");  // 10th Fibonacci number = 55
    
    test("Power function using for",
         "(do "
         "  (define (power base exp) "
         "    (do "
         "      (define result 1) "
         "      (for (i 0 exp) "
         "        (set result (* result base))) "
         "      result)) "
         "  (power 2 10))");  // 2^10 = 1024
    
    test("GCD using while",
         "(do "
         "  (define a 48) "
         "  (define b 18) "
         "  (while (> b 0) "
         "    (do "
         "      (define temp b) "
         "      (set b (% a b)) "
         "      (set a temp))) "
         "  a)");  // GCD(48, 18) = 6
    
    std::cout << "All tests completed!" << std::endl;
    
    return 0;
}
