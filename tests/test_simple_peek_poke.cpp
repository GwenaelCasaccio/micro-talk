#include "lisp_compiler.hpp"
#include "lisp_parser.hpp"
#include "stack_vm.hpp"
#include <iostream>

int main() {
    std::string code = "(do (define x 100) (poke 30000 42) (peek 30000))";

    std::cout << "=== Simple Peek/Poke Test ===" << std::endl;

    try {
        LispParser parser(code);
        auto ast = parser.parse();

        LispCompiler compiler;
        auto bytecode = compiler.compile(ast);

        std::cout << "Compiled successfully" << std::endl;

        StackVM vm;
        vm.load_program(bytecode);
        vm.execute();

        std::cout << "Result: " << vm.get_top() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
