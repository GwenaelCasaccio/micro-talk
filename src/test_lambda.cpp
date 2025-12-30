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
        
        std::cout << "Bytecode size: " << bytecode.size() << " words" << std::endl;
        
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
    std::cout << "=== Testing Lambda Functions ===" << std::endl << std::endl;
    
    // Simple functions
    std::cout << "--- Simple Functions ---" << std::endl;
    test("Simple function",
         "(do "
         "  (define (add-five x) (+ x 5)) "
         "  (add-five 10))");
    
    test("Function with two parameters",
         "(do "
         "  (define (add x y) (+ x y)) "
         "  (add 15 27))");
    
    test("Function with multiple parameters",
         "(do "
         "  (define (sum3 a b c) (+ a b c)) "
         "  (sum3 10 20 30))");
    
    test("Zero parameter function",
         "(do "
         "  (define (get-forty-two) 42) "
         "  (get-forty-two))");
    
    // Arithmetic functions
    std::cout << "--- Arithmetic Functions ---" << std::endl;
    test("Multiply function",
         "(do "
         "  (define (mul x y) (* x y)) "
         "  (mul 6 7))");
    
    test("Square function",
         "(do "
         "  (define (square x) (* x x)) "
         "  (square 9))");
    
    test("Multiple calls",
         "(do "
         "  (define (double x) (* x 2)) "
         "  (+ (double 5) (double 10)))");
    
    // Functions calling functions
    std::cout << "--- Function Composition ---" << std::endl;
    test("Function calling another function",
         "(do "
         "  (define (add x y) (+ x y)) "
         "  (define (add-ten x) (add x 10)) "
         "  (add-ten 5))");
    
    test("Nested function calls",
         "(do "
         "  (define (inc x) (+ x 1)) "
         "  (define (double x) (* x 2)) "
         "  (double (inc 5)))");
    
    // Functions with expressions
    std::cout << "--- Complex Function Bodies ---" << std::endl;
    test("Function with if",
         "(do "
         "  (define (abs x) (if (< x 0) (- 0 x) x)) "
         "  (abs -42))");
    
    test("Function with nested expressions",
         "(do "
         "  (define (formula x y) (+ (* x x) (* y y))) "
         "  (formula 3 4))");
    
    // Tagging functions
    std::cout << "--- Tagging Functions ---" << std::endl;
    test("Tag integer function",
         "(do "
         "  (define TAG_INT 1) "
         "  (define (tag-int value) (bit-or (bit-shl value 3) TAG_INT)) "
         "  (tag-int 42))");
    
    test("Untag integer function",
         "(do "
         "  (define (untag-int tagged) (bit-ashr tagged 3)) "
         "  (untag-int 337))");
    
    test("Complete tagging system",
         "(do "
         "  (define TAG_INT 1) "
         "  (define (tag-int v) (bit-or (bit-shl v 3) TAG_INT)) "
         "  (define (untag-int t) (bit-ashr t 3)) "
         "  (define (is-int? t) (= (bit-and t 7) TAG_INT)) "
         "  (define original 99) "
         "  (define tagged (tag-int original)) "
         "  (print tagged) "
         "  (define untagged (untag-int tagged)) "
         "  (print untagged) "
         "  (define check (is-int? tagged)) "
         "  (print check) "
         "  untagged)");
    
    // Recursive-style (unrolled)
    std::cout << "--- Iterative Calculations ---" << std::endl;
    test("Factorial unrolled",
         "(do "
         "  (define (fact5) "
         "    (do "
         "      (define result 1) "
         "      (set result (* result 5)) "
         "      (set result (* result 4)) "
         "      (set result (* result 3)) "
         "      (set result (* result 2)) "
         "      result)) "
         "  (fact5))");
    
    test("Function with local variables",
         "(do "
         "  (define (compute x y) "
         "    (do "
         "      (define temp (* x 2)) "
         "      (define result (+ temp y)) "
         "      result)) "
         "  (compute 10 5))");
    
    std::cout << "All tests completed!" << std::endl;
    
    return 0;
}
