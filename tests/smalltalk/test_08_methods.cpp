#include "../../src/lisp_compiler.hpp"
#include "../../src/lisp_parser.hpp"
#include "../../src/stack_vm.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

std::string read_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main() {
    std::cout << "=== Smalltalk Tests 33-43: Method Dictionary and Compilation ===" << '\n';
    std::cout << "Loading bootstrap and test code..." << '\n' << '\n';

    try {
        // Load bootstrap code
        std::string bootstrap = read_file("lisp/smalltalk/bootstrap.lisp");

        // Load test code
        std::string tests = read_file("lisp/smalltalk/test_08_methods.lisp");

        // Combine: bootstrap + tests + proper closing
        std::string code = bootstrap + tests + "\n\n      0))\n\n  (bootstrap-smalltalk))";

        // Parse
        LispParser parser(code);
        auto ast = parser.parse();

        // Compile
        LispCompiler compiler;
        auto program = compiler.compile(ast);

        std::cout << "Bytecode: " << program.bytecode.size() << " words" << '\n';
        std::cout << "Strings: " << program.strings.size() << " literals" << '\n' << '\n';

        // Execute
        StackVM vm;
        vm.load_program(program);
        vm.execute();

        std::cout << "\n=== Tests 33-43 Complete ===" << '\n';
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}
