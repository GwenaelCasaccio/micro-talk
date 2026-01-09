#include "../src/lisp_compiler.hpp"
#include "../src/lisp_parser.hpp"
#include "../src/stack_vm.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

int main() {
    std::ifstream file("lisp/smalltalk.lisp");
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string code = buffer.str();

    std::cout << "=== Smalltalk Object Model ===" << '\n';
    std::cout << "Features: Classes, Instances, Named Slots, Indexed Slots, Shapes" << '\n' << '\n';

    try {
        LispParser parser(code);
        auto ast = parser.parse();

        LispCompiler compiler;
        auto program = compiler.compile(ast);

        std::cout << "Bytecode: " << program.bytecode.size() << " words" << '\n';
        std::cout << "Strings: " << program.strings.size() << " literals" << '\n' << '\n';

        StackVM vm;
        vm.load_program(program); // Automatically handles bytecode + strings
        vm.execute();

        std::cout << "\nTest complete!" << '\n';
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
    }

    return 0;
}
