#include "stack_vm.hpp"
#include "lisp_parser.hpp"
#include "lisp_compiler.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

int main() {
    std::ifstream file("smalltalk_demo_strings.lisp");
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string code = buffer.str();
    
    std::cout << "Compiling Smalltalk demo with string literals..." << std::endl << std::endl;
    
    try {
        LispParser parser(code);
        auto ast = parser.parse();
        
        LispCompiler compiler;
        auto bytecode = compiler.compile(ast);
        
        std::cout << "Bytecode: " << bytecode.size() << " words" << std::endl << std::endl;
        
        StackVM vm;
        vm.load_program(bytecode);
        vm.execute();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    return 0;
}
