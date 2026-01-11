#include "../src/lisp_compiler.hpp"
#include "../src/lisp_parser.hpp"
#include "../src/stack_vm.hpp"
#include <cassert>
#include <iostream>

void test_radix(const std::string& code, int64_t expected, const std::string& desc) {
    std::cout << "Testing: " << desc << std::endl;

    try {
        LispParser parser(code);
        auto ast = parser.parse();

        LispCompiler compiler;
        auto program = compiler.compile(ast);

        StackVM vm;
        vm.load_program(program);
        vm.execute();

        int64_t result = vm.get_top();
        assert(result == expected);
        std::cout << "  ✓ " << code << " = " << result << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "  ✗ FAILED: " << e.what() << std::endl;
        throw;
    }
}

int main() {
    std::cout << "=== Parser Radix Notation Tests ===" << std::endl;
    std::cout << "Smalltalk-style radix notation: <radix>r<digits>" << std::endl;
    std::cout << std::endl;

    try {
        // Binary (base 2)
        std::cout << "Binary (base 2)..." << std::endl;
        test_radix("2r0", 0, "Binary zero");
        test_radix("2r1", 1, "Binary one");
        test_radix("2r1010", 10, "Binary 1010");
        test_radix("2r11111111", 255, "Binary 11111111");
        std::cout << std::endl;

        // Octal (base 8)
        std::cout << "Octal (base 8)..." << std::endl;
        test_radix("8r0", 0, "Octal zero");
        test_radix("8r77", 63, "Octal 77");
        test_radix("8r377", 255, "Octal 377");
        test_radix("8r1234", 668, "Octal 1234");
        std::cout << std::endl;

        // Decimal (base 10)
        std::cout << "Decimal (base 10)..." << std::endl;
        test_radix("10r0", 0, "Decimal zero");
        test_radix("10r42", 42, "Decimal 42");
        test_radix("10r12345", 12345, "Decimal 12345");
        std::cout << std::endl;

        // Hexadecimal (base 16)
        std::cout << "Hexadecimal (base 16)..." << std::endl;
        test_radix("16r0", 0, "Hex zero");
        test_radix("16rA", 10, "Hex A");
        test_radix("16rF", 15, "Hex F");
        test_radix("16rFF", 255, "Hex FF (uppercase)");
        test_radix("16rff", 255, "Hex ff (lowercase)");
        test_radix("16r2A", 42, "Hex 2A");
        test_radix("16rDEAD", 57005, "Hex DEAD");
        test_radix("16rBEEF", 48879, "Hex BEEF");
        test_radix("16rDEADBEEF", 3735928559, "Hex DEADBEEF");
        std::cout << std::endl;

        // Negative numbers
        std::cout << "Negative numbers..." << std::endl;
        test_radix("-2r1010", -10, "Negative binary");
        test_radix("-8r77", -63, "Negative octal");
        test_radix("-10r42", -42, "Negative decimal");
        test_radix("-16r2A", -42, "Negative hex");
        std::cout << std::endl;

        // Other bases
        std::cout << "Other bases..." << std::endl;
        test_radix("3r12", 5, "Base 3: 12 = 5");
        test_radix("5r34", 19, "Base 5: 34 = 19");
        test_radix("12rAB", 131, "Base 12: AB = 131");
        test_radix("36rZ", 35, "Base 36: Z = 35");
        test_radix("36r10", 36, "Base 36: 10 = 36");
        std::cout << std::endl;

        // In expressions
        std::cout << "In expressions..." << std::endl;
        test_radix("(+ 16rA 16r14)", 30, "Add two hex numbers");
        test_radix("(* 2r101 8r10)", 40, "Multiply binary and octal");
        test_radix("(- 16rFF 10r100)", 155, "Subtract decimal from hex");
        test_radix("(do 2r1010)", 10, "Binary in do block");
        std::cout << std::endl;

        // Case insensitivity
        std::cout << "Case insensitivity..." << std::endl;
        test_radix("16rABCD", 43981, "Uppercase hex digits");
        test_radix("16rabcd", 43981, "Lowercase hex digits");
        test_radix("16rAbCd", 43981, "Mixed case hex digits");
        std::cout << std::endl;

        std::cout << "✓ All radix notation tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Test failed: " << e.what() << std::endl;
        return 1;
    }
}
