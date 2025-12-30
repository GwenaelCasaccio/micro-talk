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
    std::cout << "=== Testing define, let, and set ===" << std::endl << std::endl;
    
    // Test define
    std::cout << "--- DEFINE Tests ---" << std::endl;
    test("Simple define", 
         "(do "
         "  (define x 42) "
         "  x)");
    
    test("Define and use",
         "(do "
         "  (define a 10) "
         "  (define b 20) "
         "  (+ a b))");
    
    test("Define with expression",
         "(do "
         "  (define x 5) "
         "  (define y (* x 2)) "
         "  (+ x y))");
    
    test("Define returns value",
         "(define result 99)");
    
    // Test set
    std::cout << "--- SET Tests ---" << std::endl;
    test("Simple set",
         "(do "
         "  (define x 10) "
         "  (set x 20) "
         "  x)");
    
    test("Set with expression",
         "(do "
         "  (define counter 0) "
         "  (set counter (+ counter 1)) "
         "  (set counter (+ counter 1)) "
         "  (set counter (+ counter 1)) "
         "  counter)");
    
    test("Set returns value",
         "(do "
         "  (define x 5) "
         "  (set x 100))");
    
    // Test let
    std::cout << "--- LET Tests ---" << std::endl;
    test("Simple let",
         "(let ((x 42)) "
         "  x)");
    
    test("Multiple bindings",
         "(let ((x 10) (y 20)) "
         "  (+ x y))");
    
    test("Let with expressions",
         "(let ((x 5) (y (* 3 4))) "
         "  (+ x y))");
    
    test("Nested let",
         "(let ((x 10)) "
         "  (let ((y 20)) "
         "    (+ x y)))");
    
    test("Let shadows outer variable",
         "(do "
         "  (define x 10) "
         "  (let ((x 20)) "
         "    (print x) "
         "    x) "
         "  x)");
    
    test("Let with multiple body expressions",
         "(let ((x 5)) "
         "  (print x) "
         "  (print (* x 2)) "
         "  (* x 3))");
    
    // Complex examples
    std::cout << "--- Complex Examples ---" << std::endl;
    test("Factorial-like calculation",
         "(do "
         "  (define n 5) "
         "  (define result 1) "
         "  (set result (* result n)) "
         "  (set n (- n 1)) "
         "  (set result (* result n)) "
         "  (set n (- n 1)) "
         "  (set result (* result n)) "
         "  (set n (- n 1)) "
         "  (set result (* result n)) "
         "  result)");
    
    test("Tagging with variables",
         "(do "
         "  (define TAG_INT 1) "
         "  (define value 42) "
         "  (define tagged (bit-or (bit-shl value 3) TAG_INT)) "
         "  (print tagged) "
         "  (define untagged (bit-ashr tagged 3)) "
         "  untagged)");
    
    test("Let with tagging",
         "(let ((TAG_INT 1) (value 99)) "
         "  (let ((tagged (bit-or (bit-shl value 3) TAG_INT))) "
         "    (let ((untagged (bit-ashr tagged 3))) "
         "      untagged)))");
    
    test("Counter with set",
         "(do "
         "  (define count 0) "
         "  (set count (+ count 10)) "
         "  (set count (+ count 20)) "
         "  (set count (+ count 30)) "
         "  count)");
    
    std::cout << "All tests completed!" << std::endl;
    
    return 0;
}
