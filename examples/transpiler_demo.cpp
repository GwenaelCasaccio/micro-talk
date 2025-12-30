#include "../src/lisp_parser.hpp"
#include "../src/lisp_to_cpp.hpp"
#include <iostream>
#include <fstream>

int main() {
    // Example 1: Simple arithmetic
    std::cout << "=== Example 1: Simple Arithmetic ===" << std::endl;
    {
        std::string lisp = "(+ (* 5 6) (- 20 8))";
        LispParser parser(lisp);
        auto ast = parser.parse();
        LispToCppCompiler compiler;
        std::string cpp = compiler.compile(ast);

        std::cout << "Lisp: " << lisp << std::endl;
        std::cout << "\nGenerated C++:\n" << cpp << std::endl;
    }

    // Example 2: Function definition
    std::cout << "\n=== Example 2: Function Definition ===" << std::endl;
    {
        std::string lisp = R"(
            (do
                (define (fibonacci n)
                    (if (< n 2)
                        n
                        (+ (fibonacci (- n 1))
                           (fibonacci (- n 2)))))
                (fibonacci 10)))";

        LispParser parser(lisp);
        auto ast = parser.parse();
        LispToCppCompiler compiler;
        std::string cpp = compiler.compile(ast);

        std::cout << "Lisp code:\n" << lisp << std::endl;
        std::cout << "\nGenerated C++:\n" << cpp << std::endl;
    }

    // Example 3: FFI demonstration
    std::cout << "\n=== Example 3: FFI (Foreign Function Interface) ===" << std::endl;
    {
        std::string lisp = "(do (define x 42) (define y -17) (c++ \"std::max(x, y)\"))";

        LispParser parser(lisp);
        auto ast = parser.parse();
        LispToCppCompiler compiler;
        std::string cpp = compiler.compile(ast);

        std::cout << "Lisp: " << lisp << std::endl;
        std::cout << "\nGenerated C++:\n" << cpp << std::endl;
    }

    // Example 4: Loops
    std::cout << "\n=== Example 4: Loops ===" << std::endl;
    {
        std::string lisp = R"(
            (do
                (define sum 0)
                (for (i 1 11)
                    (set sum (+ sum i)))
                sum))";

        LispParser parser(lisp);
        auto ast = parser.parse();
        LispToCppCompiler compiler;
        std::string cpp = compiler.compile(ast);

        std::cout << "Lisp code:\n" << lisp << std::endl;
        std::cout << "\nGenerated C++:\n" << cpp << std::endl;

        // Save to file and compile
        std::ofstream out("build/example_loop.cpp");
        out << cpp;
        out.close();

        std::cout << "\n=== Compiling and running generated code ===" << std::endl;
        system("g++ -std=c++17 -o build/example_loop build/example_loop.cpp");
        std::cout << "Output: ";
        system("./build/example_loop");
    }

    return 0;
}
