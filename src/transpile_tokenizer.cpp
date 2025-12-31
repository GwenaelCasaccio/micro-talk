#include "lisp_parser.hpp"
#include "lisp_to_cpp.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

std::string read_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + filename);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void write_file(const std::string& filename, const std::string& content) {
    std::ofstream file(filename);
    if (!file) {
        throw std::runtime_error("Failed to write file: " + filename);
    }
    file << content;
}

int main(int argc, char** argv) {
    try {
        std::string input_file = "lisp/smalltalk_tokenizer.lisp";
        std::string output_file = "build/smalltalk_tokenizer.cpp";

        if (argc > 1) {
            input_file = argv[1];
        }
        if (argc > 2) {
            output_file = argv[2];
        }

        std::cout << "Transpiling " << input_file << " -> " << output_file << std::endl;

        // Read Lisp source
        std::string lisp_code = read_file(input_file);

        // Parse
        LispParser parser(lisp_code);
        auto ast = parser.parse();

        // Transpile
        LispToCppCompiler compiler;
        std::string cpp_code = compiler.compile(ast);

        // Write output
        write_file(output_file, cpp_code);

        std::cout << "✓ Transpilation successful!" << std::endl;
        std::cout << "  C++ output: " << output_file << std::endl;
        std::cout << "\nTo compile and run:" << std::endl;
        std::cout << "  g++ -std=c++17 -o build/st_tokenizer " << output_file << std::endl;
        std::cout << "  ./build/st_tokenizer" << std::endl;

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
