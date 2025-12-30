#include "stack_vm.hpp"
#include "lisp_parser.hpp"
#include "lisp_compiler.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

int main() {
    std::ifstream file("allocator_advanced.lisp");
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string code = buffer.str();
    
    std::cout << "=== Advanced Aligned Allocator ===" << std::endl;
    std::cout << "Features: 8-byte alignment, free tracking, fragmentation metrics" << std::endl << std::endl;
    
    try {
        LispParser parser(code);
        auto ast = parser.parse();
        
        LispCompiler compiler;
        auto bytecode = compiler.compile(ast);
        
        std::cout << "Bytecode: " << bytecode.size() << " words" << std::endl << std::endl;
        
        StackVM vm;
        vm.load_program(bytecode);
        vm.execute();
        
        std::cout << "\nTest complete!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    return 0;
}
