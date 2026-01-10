#include "../src/lisp_compiler.hpp"
#include "../src/lisp_parser.hpp"
#include "../src/stack_vm.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

int main() {
    std::ifstream file("lisp/allocator_advanced.lisp");
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string code = buffer.str();

    std::cout << "=== Advanced Aligned Allocator ===" << '\n';
    std::cout << "Features: 8-byte alignment, free tracking, fragmentation metrics" << '\n' << '\n';

    try {
        LispParser parser(code);
        auto ast = parser.parse();

        LispCompiler compiler;
        auto bytecode = compiler.compile(ast);

        std::cout << "Bytecode: " << bytecode.bytecode.size() << " words" << '\n' << '\n';

        StackVM vm;
        vm.load_program(bytecode);
        vm.execute();

        std::cout << "\nTest complete!" << '\n';
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
    }

    return 0;
}
