#include "../src/lisp_compiler.hpp"
#include "../src/lisp_parser.hpp"
#include "../src/stack_vm.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

std::string read_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void run_lisp_code(const std::string& source, LispCompiler& compiler) {
    try {
        // Parse all expressions
        LispParser parser(source);
        auto exprs = parser.parse_all();

        if (exprs.empty()) {
            std::cout << "No expressions to evaluate" << '\n';
            return;
        }

        // Compile
        auto bytecode = compiler.compile_program(exprs);

        std::cout << "Compiled " << exprs.size() << " expressions into " << bytecode.size()
                  << " words of bytecode" << '\n';

        // Execute
        StackVM vm;
        vm.load_program(bytecode);
        vm.execute();

        std::cout << "Final result: " << vm.get_top() << '\n';
        std::cout << "SP: " << vm.get_sp() << '\n';

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
    }
}

int main() {
    std::cout << "=== Tagged Pointer Demo ===" << '\n' << '\n';

    LispCompiler compiler;

    // Test basic bitwise operations first
    std::cout << "--- Testing Bitwise Operations ---" << '\n';
    run_lisp_code("(bit-and 15 7)", compiler);  // 15 & 7 = 7
    run_lisp_code("(bit-or 8 4)", compiler);    // 8 | 4 = 12
    run_lisp_code("(bit-xor 15 10)", compiler); // 15 ^ 10 = 5
    run_lisp_code("(bit-shl 1 3)", compiler);   // 1 << 3 = 8
    run_lisp_code("(bit-shr 16 2)", compiler);  // 16 >> 2 = 4
    std::cout << '\n';

    // Test simple tagging operations
    std::cout << "--- Testing Integer Tagging ---" << '\n';

    // Tag integer 42: (42 << 3) | 1 = 336 | 1 = 337
    run_lisp_code("(bit-or (bit-shl 42 3) 1)", compiler);

    // Untag integer 337: 337 >> 3 = 42
    run_lisp_code("(bit-ashr 337 3)", compiler);

    // Complete round-trip
    run_lisp_code("(do "
                  "  (define TAG_INT 1)"
                  "  (define value 42)"
                  "  (define tagged (bit-or (bit-shl value 3) TAG_INT))"
                  "  (print tagged)"
                  "  (define untagged (bit-ashr tagged 3))"
                  "  untagged)",
                  compiler);
    std::cout << '\n';

    // Test OOP tagging
    std::cout << "--- Testing OOP Tagging ---" << '\n';
    run_lisp_code("(do "
                  "  (define TAG_MASK 7)"
                  "  (define address 16384)"  // Heap start, already aligned
                  "  (define tagged address)" // OOP tag is 0
                  "  (print tagged)"
                  "  (define untagged (bit-and tagged (bit-xor -1 TAG_MASK)))"
                  "  untagged)",
                  compiler);
    std::cout << '\n';

    // Test type checking
    std::cout << "--- Testing Type Checking ---" << '\n';
    run_lisp_code("(do "
                  "  (define TAG_MASK 7)"
                  "  (define TAG_INT 1)"
                  "  (define tagged-int (bit-or (bit-shl 42 3) TAG_INT))"
                  "  (define tag (bit-and tagged-int TAG_MASK))"
                  "  (print tag)"
                  "  (= tag TAG_INT))", // Should return 1 (true)
                  compiler);
    std::cout << '\n';

    // Full tagging library example
    std::cout << "--- Testing Full Tagging Functions ---" << '\n';
    run_lisp_code("(do "
                  "  (define TAG_MASK 7)"
                  "  (define TAG_INT 1)"
                  "  (define TAG_OOP 0)"
                  "  (define value 99)"
                  "  (define tag-int (bit-or (bit-shl value 3) TAG_INT))"
                  "  (print tag-int)"
                  "  (define untag-int (bit-ashr tag-int 3))"
                  "  (print untag-int)"
                  "  (define is-int (= (bit-and tag-int TAG_MASK) TAG_INT))"
                  "  (print is-int)"
                  "  (define addr 32768)"
                  "  (define tag-oop (bit-or addr TAG_OOP))"
                  "  (print tag-oop)"
                  "  (define untag-oop (bit-and tag-oop (bit-xor -1 TAG_MASK)))"
                  "  (print untag-oop)"
                  "  (define is-oop (= (bit-and tag-oop TAG_MASK) TAG_OOP))"
                  "  (print is-oop)"
                  "  999)",
                  compiler);
    std::cout << '\n';

    std::cout << "All tests completed!" << '\n';

    return 0;
}
