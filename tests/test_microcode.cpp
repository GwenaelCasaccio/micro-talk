#include "../src/microcode.hpp"
#include <iostream>

// Example microcode library for Smalltalk-like operations
namespace SmalltalkMicrocode {
// Generate standard Smalltalk microcodes
void define_smalltalk_primitives(MicrocodeCompiler& compiler) {
    // Object creation: new-object
    compiler.compile_defmicro("(defmicro new-object (class-id) "
                              "  (do "
                              "    (define-var TAG_OOP 0) "
                              "    (define-var obj-addr 16384) "
                              "    (bit-or obj-addr TAG_OOP)))");

    // Send message (simplified): send
    compiler.compile_defmicro("(defmicro send-msg (receiver selector) "
                              "  (do "
                              "    (define-var TAG_MASK 7) "
                              "    (define-var is-oop-check (= (bit-and receiver TAG_MASK) 0)) "
                              "    (if is-oop-check receiver 0)))");

    // Get class of object
    compiler.compile_defmicro("(defmicro get-class (obj) "
                              "  (do "
                              "    (define-var addr (bit-and obj (bit-xor -1 7))) "
                              "    (define-var class-id addr) "
                              "    class-id)))");

    // Array access
    compiler.compile_defmicro("(defmicro array-at (array index) "
                              "  (do "
                              "    (define-var base (bit-and array (bit-xor -1 7))) "
                              "    (define-var offset (+ base index)) "
                              "    offset)))");
}
} // namespace SmalltalkMicrocode

void test_microcode(const std::string& name, const std::string& source,
                    MicrocodeCompiler& compiler) {
    std::cout << "=== " << name << " ===" << '\n';

    try {
        auto bytecode = compiler.compile(source);

        StackVM vm;
        vm.load_program(bytecode);
        vm.execute();

        std::cout << "Result: " << vm.get_top() << '\n';
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
    }

    std::cout << '\n';
}

int main() {
    std::cout << "=== Microcode System Demo ===" << '\n' << '\n';

    MicrocodeSystem microcode_sys;
    MicrocodeCompiler compiler(microcode_sys);

    // Define some basic microcode instructions
    std::cout << "--- Defining Microcode Instructions ---" << '\n';

    // Simple arithmetic microcode
    compiler.compile_defmicro("(defmicro square (x) (* x x))");

    compiler.compile_defmicro("(defmicro cube (x) (* x (* x x)))");

    compiler.compile_defmicro("(defmicro add3 (a b c) (+ a (+ b c)))");

    // Tagged integer operations
    compiler.compile_defmicro("(defmicro tag-int (value) "
                              "  (bit-or (bit-shl value 3) 1))");

    compiler.compile_defmicro("(defmicro untag-int (tagged) "
                              "  (bit-ashr tagged 3))");

    compiler.compile_defmicro("(defmicro is-tagged-int (obj) "
                              "  (= (bit-and obj 7) 1))");

    // Tagged arithmetic
    compiler.compile_defmicro("(defmicro tagged-add (a b) "
                              "  (do "
                              "    (define-var ua (bit-ashr a 3)) "
                              "    (define-var ub (bit-ashr b 3)) "
                              "    (define-var sum (+ ua ub)) "
                              "    (bit-or (bit-shl sum 3) 1)))");

    compiler.compile_defmicro("(defmicro tagged-mul (a b) "
                              "  (do "
                              "    (define-var ua (bit-ashr a 3)) "
                              "    (define-var ub (bit-ashr b 3)) "
                              "    (define-var prod (* ua ub)) "
                              "    (bit-or (bit-shl prod 3) 1)))");

    // Object operations
    compiler.compile_defmicro("(defmicro make-oop (addr) "
                              "  (bit-or addr 0))");

    compiler.compile_defmicro("(defmicro is-oop (obj) "
                              "  (= (bit-and obj 7) 0))");

    std::cout << '\n';
    microcode_sys.print();
    std::cout << '\n';

    // Now test using these microcodes
    std::cout << "--- Testing Microcode Instructions ---" << '\n';

    // Note: These tests call the microcodes as functions
    // In a full implementation, they would be special opcodes

    test_microcode("Square",
                   "(do "
                   "  (define-func (square x) (* x x)) "
                   "  (square 9))",
                   compiler);

    test_microcode("Cube",
                   "(do "
                   "  (define-func (cube x) (* x (* x x))) "
                   "  (cube 5))",
                   compiler);

    test_microcode("Add3",
                   "(do "
                   "  (define-func (add3 a b c) (+ a (+ b c))) "
                   "  (add3 10 20 30))",
                   compiler);

    test_microcode("Tag and untag",
                   "(do "
                   "  (define-func (tag-int value) (bit-or (bit-shl value 3) 1)) "
                   "  (define-func (untag-int tagged) (bit-ashr tagged 3)) "
                   "  (define-var tagged (tag-int 42)) "
                   "  (print tagged) "
                   "  (untag-int tagged))",
                   compiler);

    test_microcode("Tagged arithmetic",
                   "(do "
                   "  (define-func (tag-int value) (bit-or (bit-shl value 3) 1)) "
                   "  (define-func (untag-int tagged) (bit-ashr tagged 3)) "
                   "  (define-func (tagged-add a b) "
                   "    (do "
                   "      (define-var ua (bit-ashr a 3)) "
                   "      (define-var ub (bit-ashr b 3)) "
                   "      (define-var sum (+ ua ub)) "
                   "      (bit-or (bit-shl sum 3) 1))) "
                   "  (define-var a (tag-int 10)) "
                   "  (define-var b (tag-int 20)) "
                   "  (define-var result (tagged-add a b)) "
                   "  (print result) "
                   "  (untag-int result))",
                   compiler);

    test_microcode("Type checking",
                   "(do "
                   "  (define-var (tag-int value) (bit-or (bit-shl value 3) 1)) "
                   "  (define-var (is-tagged-int obj) (= (bit-and obj 7) 1)) "
                   "  (define-func obj (tag-int 99)) "
                   "  (is-tagged-int obj))",
                   compiler);

    // Demonstrate Smalltalk-like primitives
    std::cout << "--- Smalltalk-like Primitives ---" << '\n';

    SmalltalkMicrocode::define_smalltalk_primitives(compiler);

    std::cout << '\n';
    microcode_sys.print();
    std::cout << '\n';

    test_microcode("Smalltalk int-add",
                   "(do "
                   "  (define-func (int-add a b) "
                   "    (do "
                   "      (define-var TAG_INT 1) "
                   "      (define-var ua (bit-ashr a 3)) "
                   "      (define-var ub (bit-ashr b 3)) "
                   "      (define-var sum (+ ua ub)) "
                   "      (bit-or (bit-shl sum 3) TAG_INT))) "
                   "  (define-var a1 (bit-or (bit-shl 15 3) 1)) "
                   "  (define-var b1 (bit-or (bit-shl 27 3) 1)) "
                   "  (define-var result1 (int-add a1 b1)) "
                   "  (bit-ashr result1 3))",
                   compiler);

    test_microcode("Smalltalk int-mul",
                   "(do "
                   "  (define-func (int-mul a b) "
                   "    (do "
                   "      (define-var TAG_INT 1) "
                   "      (define-var ua (bit-ashr a 3)) "
                   "      (define-var ub (bit-ashr b 3)) "
                   "      (define-var prod (* ua ub)) "
                   "      (bit-or (bit-shl prod 3) TAG_INT))) "
                   "  (define-var a2 (bit-or (bit-shl 6 3) 1)) "
                   "  (define-var b2 (bit-or (bit-shl 7 3) 1)) "
                   "  (define-var result2 (int-mul a2 b2)) "
                   "  (bit-ashr result2 3))",
                   compiler);

    std::cout << "=== Demo Complete ===" << '\n';
    std::cout << "\nThe microcode system allows you to:" << '\n';
    std::cout << "  1. Define new VM instructions using Lisp" << '\n';
    std::cout << "  2. Build higher-level language primitives" << '\n';
    std::cout << "  3. Implement Smalltalk-like object operations" << '\n';
    std::cout << "  4. Extend the VM without modifying C++ code" << '\n';

    return 0;
}
