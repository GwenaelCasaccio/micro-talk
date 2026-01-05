#include "lisp_compiler.hpp"
#include "lisp_parser.hpp"
#include "stack_vm.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

void test_allocator(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Could not open " << filename << std::endl;
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string code = buffer.str();

    std::cout << "=== Testing " << filename << " ===" << std::endl;

    try {
        LispParser parser(code);
        auto ast = parser.parse();

        LispCompiler compiler;
        auto bytecode = compiler.compile(ast);

        std::cout << "Bytecode size: " << bytecode.size() << " words" << std::endl;

        StackVM vm;
        vm.load_program(bytecode);
        vm.execute();

        std::cout << "Final result: " << vm.get_top() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    std::cout << std::endl;
}

int main() {
    test_allocator("allocator_basic.lisp");
    return 0;
}
