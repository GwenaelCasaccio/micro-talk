#include "stack_vm.hpp"
#include "lisp_parser.hpp"
#include "lisp_compiler.hpp"
#include <iostream>
#include <string>

void run_lisp(const std::string& source) {
    try {
        std::cout << "Source: " << source << std::endl;
        
        // Parse
        LispParser parser(source);
        auto ast = parser.parse();
        
        // Compile
        LispCompiler compiler;
        auto bytecode = compiler.compile(ast);
        
        std::cout << "Bytecode (" << bytecode.size() << " words):" << std::endl;
        for (size_t i = 0; i < bytecode.size(); i++) {
            std::cout << "  " << i << ": " << bytecode[i] << std::endl;
        }
        
        // Execute
        StackVM vm;
        vm.load_program(bytecode);
        vm.execute();
        
        std::cout << "Result: " << vm.get_top() << std::endl;
        std::cout << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "=== Minimal Stack VM + Lisp Demo ===" << std::endl << std::endl;
    
    // Basic arithmetic
    run_lisp("(+ 5 3)");
    run_lisp("(* 7 6)");
    run_lisp("(- 20 8)");
    run_lisp("(/ 100 5)");
    
    // Nested expressions
    run_lisp("(+ (* 3 4) 5)");
    run_lisp("(* (+ 2 3) (- 10 4))");
    
    // Comparison
    run_lisp("(< 5 10)");
    run_lisp("(= 7 7)");
    run_lisp("(> 3 8)");
    
    // Conditional
    run_lisp("(if (< 5 10) 100 200)");
    run_lisp("(if (> 5 10) 100 200)");
    
    // Complex expression
    run_lisp("(if (= (% 10 3) 1) (* 5 5) (+ 1 1))");
    
    // Sequential execution with print
    std::cout << "Source: (do (print 42) (print 99) (+ 1 2 3))" << std::endl;
    try {
        LispParser parser("(do (print 42) (print 99) (+ 1 2 3))");
        auto ast = parser.parse();
        
        LispCompiler compiler;
        auto bytecode = compiler.compile(ast);
        
        StackVM vm;
        vm.load_program(bytecode);
        vm.execute();
        
        std::cout << "Result: " << vm.get_top() << std::endl;
        std::cout << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    // Interactive REPL
    std::cout << "=== Interactive REPL (type 'quit' to exit) ===" << std::endl;
    std::string line;
    
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;
        
        if (line == "quit" || line == "exit") break;
        if (line.empty()) continue;
        
        run_lisp(line);
    }
    
    return 0;
}
