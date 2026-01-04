#include "stack_vm.hpp"
#include "lisp_parser.hpp"
#include "lisp_compiler.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

int main() {
    std::ifstream file("allocator_simple.lisp");
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string code = buffer.str();
    
    std::cout << "=== Testing Simple Allocator ===" << std::endl;
    
    try {
        LispParser parser(code);
        auto ast = parser.parse();
        
        LispCompiler compiler;
        auto bytecode = compiler.compile(ast);
        
        StackVM vm;
        vm.load_program(bytecode);
        vm.execute();
        
        std::cout << "Final result: " << vm.get_top() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    return 0;
}
