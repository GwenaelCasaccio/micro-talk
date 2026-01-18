#include "eval_context.hpp"
#include "lisp_compiler.hpp"
#include "lisp_parser.hpp"
#include "stack_vm.hpp"
#include "symbol_table.hpp"
#include <cassert>
#include <iostream>

// ============================================================================
// Test Harness for Runtime Eval/Compile
// ============================================================================

struct TestState {
    StackVM vm;
    SymbolTable symbols;
    uint64_t next_var_address;
    uint64_t next_string_address;
    uint64_t next_code_address;
    EvalContext eval_ctx;

    static constexpr uint64_t GLOBALS_BASE = 134217728;
    static constexpr uint64_t STRING_TABLE_START = GLOBALS_BASE;
    static constexpr uint64_t VAR_START = GLOBALS_BASE + 100000000;

    TestState()
        : next_var_address(VAR_START), next_string_address(STRING_TABLE_START),
          next_code_address(0) {
        eval_ctx.symbols = &symbols;
        eval_ctx.next_var_address = &next_var_address;
        eval_ctx.next_string_address = &next_string_address;
        eval_ctx.next_code_address = &next_code_address;

        eval_ctx.compile_for_eval = [this](StackVM& /*vm*/, const std::string& code) -> uint64_t {
            return compile_code_for_call(code);
        };

        eval_ctx.compile_for_funcall = [this](StackVM& /*vm*/,
                                              const std::string& code) -> uint64_t {
            return compile_code_for_call(code);
        };

        vm.set_eval_context(&eval_ctx);
    }

    uint64_t compile_code_for_call(const std::string& code) {
        LispParser parser(code);
        auto ast = parser.parse();

        LispCompiler compiler;
        compiler.import_symbols(symbols);
        compiler.set_next_var_address(next_var_address);
        compiler.set_next_string_address(next_string_address);
        compiler.set_code_base_address(next_code_address);

        auto program = compiler.compile_as_function(ast);

        uint64_t code_addr = next_code_address;
        vm.load_program_at(program, code_addr);
        compiler.write_strings_to_memory(vm);

        symbols.merge(compiler.get_symbol_table());
        next_var_address = compiler.get_next_var_address();
        next_string_address = compiler.get_next_string_address();
        next_code_address += program.bytecode.size();

        return code_addr;
    }

    int64_t run(const std::string& code) {
        LispParser parser(code);
        auto ast = parser.parse();

        LispCompiler compiler;
        compiler.import_symbols(symbols);
        compiler.set_next_var_address(next_var_address);
        compiler.set_next_string_address(next_string_address);
        compiler.set_code_base_address(next_code_address);

        auto program = compiler.compile(ast);

        vm.reset();
        vm.set_eval_context(&eval_ctx);
        vm.load_program_at(program, next_code_address);
        compiler.write_strings_to_memory(vm);

        // Update next_code_address BEFORE execute so that any nested
        // compile_for_eval calls don't overwrite the current program
        uint64_t start_addr = next_code_address;
        next_code_address += program.bytecode.size();

        vm.set_ip(start_addr);
        vm.execute(1000000);

        symbols.merge(compiler.get_symbol_table());
        next_var_address = compiler.get_next_var_address();
        next_string_address = compiler.get_next_string_address();

        return static_cast<int64_t>(vm.get_top());
    }
};

// Test counter
int tests_passed = 0;
int tests_failed = 0;

void check(bool condition, const std::string& test_name) {
    if (condition) {
        std::cout << "  PASS: " << test_name << std::endl;
        tests_passed++;
    } else {
        std::cout << "  FAIL: " << test_name << std::endl;
        tests_failed++;
    }
}

int main() {
    std::cout << "=== Testing Runtime Eval/Compile ===" << std::endl << std::endl;

    try {
        // Test 1: Simple eval with arithmetic
        std::cout << "Test 1: Simple eval with arithmetic" << std::endl;
        {
            TestState state;
            int64_t result = state.run("(eval \"(+ 1 2)\")");
            check(result == 3, "eval \"(+ 1 2)\" should return 3");
        }

        // Test 2: Eval with nested expressions
        std::cout << "\nTest 2: Eval with nested expressions" << std::endl;
        {
            TestState state;
            int64_t result = state.run("(eval \"(* (+ 2 3) 4)\")");
            check(result == 20, "eval \"(* (+ 2 3) 4)\" should return 20");
        }

        // Test 3: Compile and funcall
        std::cout << "\nTest 3: Compile and funcall" << std::endl;
        {
            TestState state;
            // Define a code string, compile it, call it with funcall (no args)
            int64_t result = state.run("(do "
                                       "  (define-var code \"(* 6 7)\") "
                                       "  (define-var fn (compile code)) "
                                       "  (funcall fn))");
            check(result == 42, "compile and funcall \"(* 6 7)\" should return 42");
        }

        // Test 4: Eval defines variable that persists
        std::cout << "\nTest 4: Eval defines variable that persists" << std::endl;
        {
            TestState state;
            state.run("(eval \"(define-var x 100)\")");
            int64_t result = state.run("x");
            check(result == 100, "variable x defined in eval should persist");
        }

        // Test 5: Compile multiple expressions
        std::cout << "\nTest 5: Compile multiple expressions" << std::endl;
        {
            TestState state;
            int64_t result = state.run("(do "
                                       "  (define-var add5 (compile \"(+ 5 10)\")) "
                                       "  (define-var mul3 (compile \"(* 3 4)\")) "
                                       "  (+ (funcall add5) (funcall mul3)))");
            check(result == 27, "15 + 12 = 27");
        }

        // Test 6: Eval with comparison
        std::cout << "\nTest 6: Eval with comparison" << std::endl;
        {
            TestState state;
            int64_t result = state.run("(eval \"(< 5 10)\")");
            check(result == 1, "eval \"(< 5 10)\" should return 1 (true)");
        }

        // Test 7: Eval with conditional
        std::cout << "\nTest 7: Eval with conditional" << std::endl;
        {
            TestState state;
            int64_t result = state.run("(eval \"(if (< 1 2) 100 200)\")");
            check(result == 100, "eval \"(if (< 1 2) 100 200)\" should return 100");
        }

        // Test 8: Eval with bitwise operations
        std::cout << "\nTest 8: Eval with bitwise operations" << std::endl;
        {
            TestState state;
            int64_t result = state.run("(eval \"(bit-or 1 2)\")");
            check(result == 3, "eval \"(bit-or 1 2)\" should return 3");
        }

        std::cout << std::endl << "=== Summary ===" << std::endl;
        std::cout << "Passed: " << tests_passed << std::endl;
        std::cout << "Failed: " << tests_failed << std::endl;

        if (tests_failed > 0) {
            return 1;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    std::cout << std::endl << "=== All Eval/Compile Tests Passed! ===" << std::endl;
    return 0;
}
