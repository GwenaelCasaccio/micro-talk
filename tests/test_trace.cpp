#include "stack_vm.hpp"
#include "lisp_parser.hpp"
#include "lisp_compiler.hpp"
#include <iostream>

int main() {
    std::string code = "(do (define (square x) (* x x)) (square 5))";
    
    LispParser parser(code);
    auto ast = parser.parse();
    
    LispCompiler compiler;
    auto bytecode = compiler.compile(ast);
    
    std::cout << "Bytecode:" << std::endl;
    for (size_t i = 0; i < bytecode.size() && i < 30; i++) {
        std::cout << i << ": " << bytecode[i] << std::endl;
    }
    
    std::cout << "\nExecuting..." << std::endl;
    
    StackVM vm;
    vm.load_program(bytecode);
    try {
        vm.execute();
        std::cout << "Result: " << vm.get_top() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    return 0;
}
