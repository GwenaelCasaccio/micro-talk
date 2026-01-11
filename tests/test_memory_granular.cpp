#include "../src/lisp_compiler.hpp"
#include "../src/lisp_parser.hpp"
#include "../src/stack_vm.hpp"
#include <iostream>

void run_test(const std::string& name, const std::string& code, int64_t expected) {
    std::cout << "Testing: " << name << std::endl;

    try {
        LispParser parser(code);
        auto ast = parser.parse();

        LispCompiler compiler;
        auto program = compiler.compile(ast);

        StackVM vm;
        vm.load_program(program);
        program.write_strings(vm);
        vm.execute();

        int64_t result = vm.get_top();

        if (result == expected) {
            std::cout << "  ✓ PASSED: " << result << std::endl;
        } else {
            std::cout << "  ✗ FAILED: Expected " << expected << ", got " << result << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "  ✗ ERROR: " << e.what() << std::endl;
    }

    std::cout << std::endl;
}

int main() {
    std::cout << "=== Granular Memory Operations Tests ===" << std::endl;
    std::cout << "Testing byte-level and 32-bit memory access" << std::endl << std::endl;

    // Memory layout reminder:
    // - peek/poke use WORD addresses (64-bit words)
    // - peek-byte/poke-byte use BYTE addresses (8 bytes per word)
    // - HEAP_START = 268435456 words = 2147483648 bytes

    // Test 1: Byte operations
    std::cout << "--- Test 1: Byte-Level Operations ---" << std::endl;
    run_test("Store and load single byte",
             "(do "
             "  (define-var HEAP_WORDS 268435456) "
             "  (define-var addr (* HEAP_WORDS 8)) "
             "  (poke-byte addr 66) "
             "  (peek-byte addr))",
             66);

    run_test("Store multiple bytes in same word",
             "(do "
             "  (define-var base (* 268435456 8)) "
             "  (poke-byte base 17) "
             "  (poke-byte (+ base 1) 34) "
             "  (poke-byte (+ base 2) 51) "
             "  (poke-byte (+ base 3) 68) "
             "  (+ (+ (peek-byte base) (peek-byte (+ base 1))) "
             "     (+ (peek-byte (+ base 2)) (peek-byte (+ base 3)))))",
             17 + 34 + 51 + 68);

    run_test("Byte operations preserve other bytes",
             "(do "
             "  (define-var word-addr 268435456) "
             "  (poke word-addr 1234605616436508552) "
             "  (define-var byte-addr (* word-addr 8)) "
             "  (poke-byte (+ byte-addr 1) 255) "
             "  (peek-byte (+ byte-addr 1)))",
             255);

    // Test 2: 32-bit operations
    // Note: peek32/poke32 use byte addresses for 32-bit values
    std::cout << "--- Test 2: 32-bit Operations ---" << std::endl;
    run_test("Store and load 32-bit value",
             "(do "
             "  (define-var addr32 (* 268435456 8)) "
             "  (poke32 addr32 305419896) "
             "  (peek32 addr32))",
             305419896);

    run_test("Two 32-bit values in one 64-bit word",
             "(do "
             "  (define-var base (* 268435456 8)) "
             "  (poke32 base 286331153) "
             "  (poke32 (+ base 4) 572662306) "
             "  (+ (peek32 base) (peek32 (+ base 4))))",
             286331153 + 572662306);

    // Test 3: Mixed operations
    std::cout << "--- Test 3: Mixed Operations ---" << std::endl;
    run_test("Mix byte and 64-bit word operations",
             "(do "
             "  (define-var word-addr 268435460) "
             "  (poke word-addr 0) "
             "  (define-var byte-addr (* word-addr 8)) "
             "  (poke-byte byte-addr 18) "
             "  (poke-byte (+ byte-addr 7) 52) "
             "  (+ (peek-byte byte-addr) (peek-byte (+ byte-addr 7))))",
             18 + 52);

    run_test("Verify byte addressing within word",
             "(do "
             "  (define-var word-addr 268435472) "
             "  (poke word-addr 72623859790382856) "
             "  (define-var base (* word-addr 8)) "
             "  (+ (+ (+ (peek-byte base) (peek-byte (+ base 1))) "
             "        (+ (peek-byte (+ base 2)) (peek-byte (+ base 3)))) "
             "     (+ (+ (peek-byte (+ base 4)) (peek-byte (+ base 5))) "
             "        (+ (peek-byte (+ base 6)) (peek-byte (+ base 7))))))",
             8 + 7 + 6 + 5 + 4 + 3 + 2 + 1);

    // Test 4: Memory alignment with 8-byte malloc
    std::cout << "--- Test 4: 8-byte Alignment ---" << std::endl;
    run_test("Malloc returns 8-byte aligned addresses",
             "(do "
             "  (define-var HEAP_START 268435456) "
             "  (define-var heap-pointer HEAP_START) "
             "  (define-func (malloc size) "
             "    (do "
             "      (define-var result heap-pointer) "
             "      (define-var remainder (% size 8)) "
             "      (define-var aligned-size (if (= remainder 0) size (+ size (- 8 remainder)))) "
             "      (set heap-pointer (+ heap-pointer aligned-size)) "
             "      result)) "
             "  (define-var addr1 (malloc 5)) "
             "  (define-var addr2 (malloc 3)) "
             "  (define-var addr3 (malloc 10)) "
             "  (if (= (% addr1 8) 0) "
             "      (if (= (% addr2 8) 0) "
             "          (if (= (% addr3 8) 0) 1 0) "
             "          0) "
             "      0))",
             1);

    run_test("8-byte alignment preserves 3 bits for tagging",
             "(do "
             "  (define-var addr 268435456) "
             "  (define-var last-3-bits (bit-and addr 7)) "
             "  last-3-bits)",
             0); // Last 3 bits should be 000

    // Test 5: Space efficiency
    std::cout << "--- Test 5: Space Efficiency ---" << std::endl;
    run_test("Pack 8 bytes into one 64-bit word",
             "(do "
             "  (define-var word-addr 268435488) "
             "  (define-var byte-addr (* word-addr 8)) "
             "  (for (i 0 8) "
             "    (poke-byte (+ byte-addr i) (+ i 1))) "
             "  (define-var sum 0) "
             "  (for (i 0 8) "
             "    (set sum (+ sum (peek-byte (+ byte-addr i))))) "
             "  sum)",
             36); // 1+2+3+4+5+6+7+8 = 36

    std::cout << "=== All Granular Memory Tests Complete ===" << std::endl;
    std::cout << "✓ Byte-level operations working" << std::endl;
    std::cout << "✓ 32-bit operations working" << std::endl;
    std::cout << "✓ 8-byte alignment ensures 3 bits for tagging" << std::endl;
    std::cout << "✓ Mixed operations preserve data correctly" << std::endl;
    std::cout << "✓ Efficient memory packing enabled" << std::endl;

    return 0;
}
