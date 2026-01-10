#include "disassembler.hpp"
#include "lisp_compiler.hpp"
#include "lisp_parser.hpp"
#include <iostream>

int main() {
    std::cout << "=== Disassembler Test ===" << '\n' << '\n';

    // Test 1: Simple arithmetic
    std::cout << "Test 1: Simple arithmetic (+ 5 3)" << '\n';
    std::cout << "-----------------------------------" << '\n';
    {
        LispCompiler compiler;
        LispParser parser("(+ 5 3)");
        auto ast = parser.parse();
        auto bytecode = compiler.compile(ast);

        std::cout << "Lisp code: (+ 5 3)" << '\n';
        std::cout << "Disassembly:" << '\n';
        Disassembler::disassemble(bytecode.bytecode);
        std::cout << '\n';
    }

    // Test 2: Conditional with if
    std::cout << "Test 2: Conditional (if (> 10 5) 42 0)" << '\n';
    std::cout << "----------------------------------------" << '\n';
    {
        LispCompiler compiler;
        LispParser parser("(if (> 10 5) 42 0)");
        auto ast = parser.parse();
        auto bytecode = compiler.compile(ast);

        std::cout << "Lisp code: (if (> 10 5) 42 0)" << '\n';
        std::cout << "Disassembly:" << '\n';
        Disassembler::disassemble(bytecode.bytecode);
        std::cout << '\n';
    }

    // Test 3: Variable definition and arithmetic
    std::cout << "Test 3: Variables (do (define-var x 10) (+ x 5))" << '\n';
    std::cout << "-------------------------------------------------" << '\n';
    {
        LispCompiler compiler;
        LispParser parser("(do (define-var x 10) (+ x 5))");
        auto ast = parser.parse();
        auto bytecode = compiler.compile(ast);

        std::cout << "Lisp code: (do (define-var x 10) (+ x 5))" << '\n';
        std::cout << "Disassembly:" << '\n';
        Disassembler::disassemble(bytecode.bytecode);
        std::cout << '\n';
    }

    // Test 4: While loop
    std::cout << "Test 4: While loop (do (define-var i 0) (while (< i 3) (set i (+ i 1))))" << '\n';
    std::cout << "--------------------------------------------------------------------------"
              << '\n';
    {
        LispCompiler compiler;
        LispParser parser("(do (define-var i 0) (while (< i 3) (set i (+ i 1))))");
        auto ast = parser.parse();
        auto bytecode = compiler.compile(ast);

        std::cout << "Lisp code: (do (define-var i 0) (while (< i 3) (set i (+ i 1))))" << '\n';
        std::cout << "Disassembly:" << '\n';
        Disassembler::disassemble(bytecode.bytecode);
        std::cout << '\n';
    }

    // Test 5: Bitwise operations (for tagged pointers)
    std::cout << "Test 5: Bitwise operations (bit-or (bit-shl 42 3) 1)" << '\n';
    std::cout << "-----------------------------------------------------" << '\n';
    {
        LispCompiler compiler;
        LispParser parser("(bit-or (bit-shl 42 3) 1)");
        auto ast = parser.parse();
        auto bytecode = compiler.compile(ast);

        std::cout << "Lisp code: (bit-or (bit-shl 42 3) 1)" << '\n';
        std::cout << "Disassembly:" << '\n';
        Disassembler::disassemble(bytecode.bytecode);
        std::cout << '\n';
    }

    // Test 6: Nested expressions
    std::cout << "Test 6: Nested expressions (* (+ 2 3) (- 10 4))" << '\n';
    std::cout << "------------------------------------------------" << '\n';
    {
        LispCompiler compiler;
        LispParser parser("(* (+ 2 3) (- 10 4))");
        auto ast = parser.parse();
        auto bytecode = compiler.compile(ast);

        std::cout << "Lisp code: (* (+ 2 3) (- 10 4))" << '\n';
        std::cout << "Disassembly:" << '\n';
        Disassembler::disassemble(bytecode.bytecode);
        std::cout << '\n';
    }

    std::cout << "=== All tests complete ===" << '\n';
    return 0;
}
