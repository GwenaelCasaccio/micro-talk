#include "disassembler.hpp"
#include "lisp_compiler.hpp"
#include "lisp_parser.hpp"
#include "stack_vm.hpp"
#include <fstream>
#include <iostream>
#include <string>

void run_lisp(const std::string& source) {
    try {
        std::cout << "Source: " << source << '\n';

        // Parse
        LispParser parser(source);
        auto ast = parser.parse();

        // Compile
        LispCompiler compiler;
        auto bytecode = compiler.compile(ast);

        std::cout << "Bytecode (" << bytecode.size() << " words):" << '\n';
        for (size_t i = 0; i < bytecode.size(); i++) {
            std::cout << "  " << i << ": " << bytecode[i] << '\n';
        }

        // Execute
        StackVM vm;
        vm.load_program(bytecode);
        vm.execute();

        std::cout << "Result: " << vm.get_top() << '\n';
        std::cout << '\n';

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
    }
}

int main() {
    std::cout << "=== Smalltalk ===" << '\n' << '\n';

    std::ifstream file("lisp/smalltalk.lisp");
    if (!file.is_open()) {
        std::cerr << "Cannot open file" << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string code = buffer.str();

    std::cout << "Compiling Smalltalk..." << std::endl << std::endl;

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
        return 2;
    }

    // Interactive REPL
    std::cout << "=== Interactive REPL (type 'quit' to exit) ===" << '\n';
    std::string line;

    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line))
            break;

        if (line == "quit" || line == "exit")
            break;
        if (line.empty())
            continue;

        run_lisp(line);
    }

    return 0;
}
