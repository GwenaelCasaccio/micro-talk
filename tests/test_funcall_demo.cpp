#include "../src/lisp_compiler.hpp"
#include "../src/lisp_parser.hpp"
#include "../src/stack_vm.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

int main() {
    std::cout << "=== funcall Demonstration ===" << std::endl;
    std::cout << std::endl;

    try {
        // Load the funcall demo
        std::ifstream file("lisp/funcall_demo.lisp");
        if (!file) {
            std::cerr << "Error: Could not open lisp/funcall_demo.lisp" << std::endl;
            return 1;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string code = buffer.str();

        // Parse and compile
        LispParser parser(code);
        auto ast = parser.parse();

        LispCompiler compiler;
        auto program = compiler.compile(ast);

        std::cout << "Compiled " << program.bytecode.size() << " words of bytecode" << std::endl;
        std::cout << "String literals: " << program.strings.size() << std::endl;
        std::cout << std::endl;

        // Execute
        StackVM vm;
        vm.load_program(program);
        vm.execute();

        std::cout << std::endl;
        std::cout << "=== Demo complete! ===" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
