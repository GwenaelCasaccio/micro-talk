#include "stack_vm.hpp"
#include "lisp_parser.hpp"
#include "lisp_compiler.hpp"
#include <iostream>

void eval(const std::string& expr) {
    try {
        std::cout << "Expression: " << expr << std::endl;
        
        LispParser parser(expr);
        auto ast = parser.parse();
        
        LispCompiler compiler;
        auto bytecode = compiler.compile(ast);
        
        StackVM vm;
        vm.load_program(bytecode);
        vm.execute();
        
        std::cout << "Result: " << vm.get_top() << std::endl;
        std::cout << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl << std::endl;
    }
}

int main() {
    std::cout << "=== Lisp Tagged Pointer Functions ===" << std::endl << std::endl;
    
    // Constants
    std::cout << "--- Constants ---" << std::endl;
    std::cout << "TAG_MASK = 7, TAG_INT = 1, TAG_OOP = 0" << std::endl << std::endl;
    
    // Integer tagging
    std::cout << "--- Tag Integer: (value << 3) | 1 ---" << std::endl;
    eval("(bit-or (bit-shl 42 3) 1)");        // 42 -> 337
    eval("(bit-or (bit-shl 99 3) 1)");        // 99 -> 793
    eval("(bit-or (bit-shl 0 3) 1)");         // 0 -> 1
    eval("(bit-or (bit-shl -42 3) 1)");       // -42 -> -335
    
    // Integer untagging
    std::cout << "--- Untag Integer: tagged >> 3 ---" << std::endl;
    eval("(bit-ashr 337 3)");                 // 337 -> 42
    eval("(bit-ashr 793 3)");                 // 793 -> 99
    eval("(bit-ashr 1 3)");                   // 1 -> 0
    eval("(bit-ashr -335 3)");                // -335 -> -42
    
    // Round-trip
    std::cout << "--- Integer Round-trip ---" << std::endl;
    eval("(bit-ashr (bit-or (bit-shl 12345 3) 1) 3)");  // Should return 12345
    
    // OOP tagging
    std::cout << "--- Tag OOP: addr | 0 ---" << std::endl;
    eval("(bit-or 16384 0)");                 // Heap start
    eval("(bit-or 32768 0)");                 // Middle of heap
    eval("(bit-or 49152 0)");                 // Upper heap
    
    // OOP untagging
    std::cout << "--- Untag OOP: tagged & ~7 ---" << std::endl;
    eval("(bit-and 16384 (bit-xor -1 7))");   // 16384 -> 16384
    eval("(bit-and 32768 (bit-xor -1 7))");   // 32768 -> 32768
    
    // Type checking - is-int?
    std::cout << "--- Type Check: is-int? (tag & 7) == 1 ---" << std::endl;
    eval("(= (bit-and 337 7) 1)");            // Tagged int -> 1 (true)
    eval("(= (bit-and 793 7) 1)");            // Tagged int -> 1 (true)
    eval("(= (bit-and 16384 7) 1)");          // OOP -> 0 (false)
    
    // Type checking - is-oop?
    std::cout << "--- Type Check: is-oop? (tag & 7) == 0 ---" << std::endl;
    eval("(= (bit-and 16384 7) 0)");          // OOP -> 1 (true)
    eval("(= (bit-and 32768 7) 0)");          // OOP -> 1 (true)
    eval("(= (bit-and 337 7) 0)");            // Tagged int -> 0 (false)
    
    // Extract tag bits
    std::cout << "--- Extract Tag: tagged & 7 ---" << std::endl;
    eval("(bit-and 337 7)");                  // Tagged int -> 1
    eval("(bit-and 793 7)");                  // Tagged int -> 1
    eval("(bit-and 16384 7)");                // OOP -> 0
    eval("(bit-and 32768 7)");                // OOP -> 0
    
    // Complete demo with prints
    std::cout << "--- Complete Demo with Trace ---" << std::endl;
    eval("(do "
         "  (print 42) "
         "  (print (bit-or (bit-shl 42 3) 1)) "
         "  (print (bit-ashr 337 3)) "
         "  (print (= (bit-and 337 7) 1)) "
         "  999)");
    
    std::cout << "All tests completed!" << std::endl;
    
    return 0;
}
