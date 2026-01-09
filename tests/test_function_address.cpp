#include "../src/lisp_compiler.hpp"
#include "../src/lisp_parser.hpp"
#include "../src/stack_vm.hpp"
#include <iostream>

int main() {
    std::string code = R"(
        (do
            (define-func (add-two x y)
                (+ x y))

            (define-func (multiply x y)
                (* x y))

            ; Get function addresses
            (define-var add-addr (function-address add-two))
            (define-var mul-addr (function-address multiply))

            ; Print the addresses
            (print add-addr)
            (print mul-addr)

            ; Verify addresses are different and non-zero
            (if (= add-addr 0)
                (abort "add-addr is zero!")
                0)

            (if (= mul-addr 0)
                (abort "mul-addr is zero!")
                0)

            (if (= add-addr mul-addr)
                (abort "addresses are the same!")
                0)

            ; Success
            (print 42)))
    )";

    std::cout << "=== Testing function-address ===" << std::endl;

    try {
        LispParser parser(code);
        auto ast = parser.parse();

        LispCompiler compiler;
        auto bytecode = compiler.compile(ast);

        std::cout << "Bytecode: " << bytecode.size() << " words" << std::endl;

        StackVM vm;
        vm.load_program(bytecode);
        vm.execute();

        std::cout << "\nTest passed!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
