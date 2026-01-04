#include "stack_vm.hpp"
#include "lisp_parser.hpp"
#include "lisp_compiler.hpp"
#include <iostream>

int main() {
    // Simplest possible function call
    std::string code = "(do (define (id x) x) (id 42))";
    
    LispParser parser(code);
    auto ast = parser.parse();
    
    LispCompiler compiler;
    auto bytecode = compiler.compile(ast);
    
    std::cout << "Bytecode:" << std::endl;
    for (size_t i = 0; i < bytecode.size(); i++) {
        std::cout << i << ": " << bytecode[i];
        if (i == 5) std::cout << " <- CALL";
        if (i == 6) std::cout << " <- function addr";
        if (i == 7) std::cout << " <- HALT"; 
        if (i == 8) std::cout << " <- function start";
        std::cout << std::endl;
    }
    
    StackVM vm;
    vm.load_program(bytecode);
    try {
        vm.execute();
        std::cout << "\nResult: " << vm.get_top() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "\nError: " << e.what() << std::endl;
    }
    
    return 0;
}
