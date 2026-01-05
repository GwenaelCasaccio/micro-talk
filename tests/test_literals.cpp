#include "lisp_compiler.hpp"
#include "lisp_parser.hpp"
#include "stack_vm.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

int main() {
    std::ifstream file("test_string_literals.lisp");
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string code = buffer.str();

    std::cout << "=== String Literals Test ===" << std::endl;
    std::cout << "Using \"quoted strings\" instead of ASCII codes!" << std::endl << std::endl;

    try {
        LispParser parser(code);
        auto ast = parser.parse();

        LispCompiler compiler;
        auto bytecode = compiler.compile(ast);

        std::cout << "Bytecode: " << bytecode.size() << " words" << std::endl << std::endl;

        StackVM vm;
        vm.load_program(bytecode);
        vm.execute();

        std::cout << "\nSuccess!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
