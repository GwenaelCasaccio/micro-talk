#include "microcode.hpp"
#include <iostream>

void test_microcode(const std::string& name, const std::string& source, MicrocodeCompiler& compiler) {
    std::cout << "=== " << name << " ===" << std::endl;
    
    try {
        auto bytecode = compiler.compile(source);
        
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
    std::cout << "=== Microcode System Demo ===" << std::endl << std::endl;
    
    MicrocodeSystem microcode_sys;
    MicrocodeCompiler compiler(microcode_sys);
    
    // Define some basic microcode instructions
    std::cout << "--- Defining Microcode Instructions ---" << std::endl;
    
    // Simple arithmetic microcode
    compiler.compile_defmicro(
        "(defmicro square (x) (* x x))");
    
    compiler.compile_defmicro(
        "(defmicro cube (x) (* x (* x x)))");
    
    compiler.compile_defmicro(
        "(defmicro add3 (a b c) (+ a (+ b c)))");
    
    // Tagged integer operations
    compiler.compile_defmicro(
        "(defmicro tag-int (value) "
        "  (bit-or (bit-shl value 3) 1))");
    
    compiler.compile_defmicro(
        "(defmicro untag-int (tagged) "
        "  (bit-ashr tagged 3))");
    
    compiler.compile_defmicro(
        "(defmicro is-tagged-int (obj) "
        "  (= (bit-and obj 7) 1))");
    
    // Tagged arithmetic
    compiler.compile_defmicro(
        "(defmicro tagged-add (a b) "
        "  (do "
        "    (define ua (bit-ashr a 3)) "
        "    (define ub (bit-ashr b 3)) "
        "    (define sum (+ ua ub)) "
        "    (bit-or (bit-shl sum 3) 1)))");
    
    compiler.compile_defmicro(
        "(defmicro tagged-mul (a b) "
        "  (do "
        "    (define ua (bit-ashr a 3)) "
        "    (define ub (bit-ashr b 3)) "
        "    (define prod (* ua ub)) "
        "    (bit-or (bit-shl prod 3) 1)))");
    
    // Object operations
    compiler.compile_defmicro(
        "(defmicro make-oop (addr) "
        "  (bit-or addr 0))");
    
    compiler.compile_defmicro(
        "(defmicro is-oop (obj) "
        "  (= (bit-and obj 7) 0))");
    
    std::cout << std::endl;
    microcode_sys.print();
    std::cout << std::endl;
    
    // Now test using these microcodes
    std::cout << "--- Testing Microcode Instructions ---" << std::endl;
    
    // Note: These tests call the microcodes as functions
    // In a full implementation, they would be special opcodes
    
    test_microcode("Square",
        "(do "
        "  (define (square x) (* x x)) "
        "  (square 9))",
        compiler);
    
    test_microcode("Cube",
        "(do "
        "  (define (cube x) (* x (* x x))) "
        "  (cube 5))",
        compiler);
    
    test_microcode("Add3",
        "(do "
        "  (define (add3 a b c) (+ a (+ b c))) "
        "  (add3 10 20 30))",
        compiler);
    
    test_microcode("Tag and untag",
        "(do "
        "  (define (tag-int value) (bit-or (bit-shl value 3) 1)) "
        "  (define (untag-int tagged) (bit-ashr tagged 3)) "
        "  (define tagged (tag-int 42)) "
        "  (print tagged) "
        "  (untag-int tagged))",
        compiler);
    
    test_microcode("Tagged arithmetic",
        "(do "
        "  (define (tag-int value) (bit-or (bit-shl value 3) 1)) "
        "  (define (untag-int tagged) (bit-ashr tagged 3)) "
        "  (define (tagged-add a b) "
        "    (do "
        "      (define ua (bit-ashr a 3)) "
        "      (define ub (bit-ashr b 3)) "
        "      (define sum (+ ua ub)) "
        "      (bit-or (bit-shl sum 3) 1))) "
        "  (define a (tag-int 10)) "
        "  (define b (tag-int 20)) "
        "  (define result (tagged-add a b)) "
        "  (print result) "
        "  (untag-int result))",
        compiler);
    
    test_microcode("Type checking",
        "(do "
        "  (define (tag-int value) (bit-or (bit-shl value 3) 1)) "
        "  (define (is-tagged-int obj) (= (bit-and obj 7) 1)) "
        "  (define obj (tag-int 99)) "
        "  (is-tagged-int obj))",
        compiler);
    
    // Demonstrate Smalltalk-like primitives
    std::cout << "--- Smalltalk-like Primitives ---" << std::endl;
    
    SmalltalkMicrocode::define_smalltalk_primitives(compiler);
    
    std::cout << std::endl;
    microcode_sys.print();
    std::cout << std::endl;
    
    test_microcode("Smalltalk int-add",
        "(do "
        "  (define (int-add a b) "
        "    (do "
        "      (define TAG_INT 1) "
        "      (define ua (bit-ashr a 3)) "
        "      (define ub (bit-ashr b 3)) "
        "      (define sum (+ ua ub)) "
        "      (bit-or (bit-shl sum 3) TAG_INT))) "
        "  (define a (bit-or (bit-shl 15 3) 1)) "
        "  (define b (bit-or (bit-shl 27 3) 1)) "
        "  (define result (int-add a b)) "
        "  (bit-ashr result 3))",
        compiler);
    
    test_microcode("Smalltalk int-mul",
        "(do "
        "  (define (int-mul a b) "
        "    (do "
        "      (define TAG_INT 1) "
        "      (define ua (bit-ashr a 3)) "
        "      (define ub (bit-ashr b 3)) "
        "      (define prod (* ua ub)) "
        "      (bit-or (bit-shl prod 3) TAG_INT))) "
        "  (define a (bit-or (bit-shl 6 3) 1)) "
        "  (define b (bit-or (bit-shl 7 3) 1)) "
        "  (define result (int-mul a b)) "
        "  (bit-ashr result 3))",
        compiler);
    
    std::cout << "=== Demo Complete ===" << std::endl;
    std::cout << "\nThe microcode system allows you to:" << std::endl;
    std::cout << "  1. Define new VM instructions using Lisp" << std::endl;
    std::cout << "  2. Build higher-level language primitives" << std::endl;
    std::cout << "  3. Implement Smalltalk-like object operations" << std::endl;
    std::cout << "  4. Extend the VM without modifying C++ code" << std::endl;
    
    return 0;
}
