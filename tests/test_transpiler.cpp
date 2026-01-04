#include "../src/lisp_parser.hpp"
#include "../src/lisp_to_cpp.hpp"
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>

void save_cpp_file(const std::string& filename, const std::string& code) {
    std::ofstream out(filename);
    if (!out) {
        throw std::runtime_error("Failed to open file: " + filename);
    }
    out << code;
    out.close();
}

std::string run_command(const std::string& cmd) {
    FILE* pipe = popen(cmd.c_str(), "r");
    if (pipe == nullptr) {
        throw std::runtime_error("Failed to run command: " + cmd);
    }

    char buffer[128];
    std::string result;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }

    int status = pclose(pipe);
    if (status != 0) {
        throw std::runtime_error("Command failed with status " + std::to_string(status) + ": " +
                                 cmd);
    }

    return result;
}

void test_transpile_and_run(const std::string& name, const std::string& lisp_code,
                            const std::string& expected_output) {
    std::cout << "Testing " << name << "..." << '\n';

    // Parse
    LispParser parser(lisp_code);
    auto ast = parser.parse();

    // Transpile
    LispToCppCompiler compiler;
    std::string cpp_code = compiler.compile(ast);

    // Save to file
    std::string cpp_file = "build/test_" + name + ".cpp";
    std::string exe_file = "build/test_" + name;
    save_cpp_file(cpp_file, cpp_code);

    // Compile
    std::string compile_cmd = "g++ -std=c++17 -o " + exe_file + " " + cpp_file + " 2>&1";
    try {
        run_command(compile_cmd);
    } catch (const std::exception& e) {
        std::cerr << "Compilation failed for " << name << '\n';
        std::cerr << "C++ code:\n" << cpp_code << '\n';
        throw;
    }

    // Run
    std::string run_cmd = "./" + exe_file;
    std::string output = run_command(run_cmd);

    // Remove trailing newline
    if (!output.empty() && output.back() == '\n') {
        output.pop_back();
    }

    // Verify
    if (output != expected_output) {
        std::cerr << "Expected: " << expected_output << '\n';
        std::cerr << "Got: " << output << '\n';
        std::cerr << "C++ code:\n" << cpp_code << '\n';
        throw std::runtime_error("Output mismatch for " + name);
    }

    std::cout << "  ✓ " << name << ": " << output << '\n';
}

void test_simple_arithmetic() {
    test_transpile_and_run("add", "(+ 10 20)", "30");
    test_transpile_and_run("sub", "(- 50 20)", "30");
    test_transpile_and_run("mul", "(* 6 7)", "42");
    test_transpile_and_run("div", "(/ 100 5)", "20");
    test_transpile_and_run("mod", "(% 17 5)", "2");
}

void test_multi_arg_arithmetic() {
    test_transpile_and_run("multi_add", "(+ 1 2 3 4)", "10");
    test_transpile_and_run("multi_mul", "(* 2 3 4)", "24");
}

void test_comparison() {
    test_transpile_and_run("eq_true", "(= 42 42)", "1");
    test_transpile_and_run("eq_false", "(= 42 100)", "0");
    test_transpile_and_run("lt_true", "(< 10 20)", "1");
    test_transpile_and_run("lt_false", "(< 20 10)", "0");
    test_transpile_and_run("gt_true", "(> 20 10)", "1");
    test_transpile_and_run("gt_false", "(> 10 20)", "0");
}

void test_nested_expressions() {
    test_transpile_and_run("nested1", "(+ (* 2 3) 4)", "10");
    test_transpile_and_run("nested2", "(* (+ 2 3) (- 10 4))", "30");
}

void test_variables() {
    test_transpile_and_run("define_var", "(do (define-var x 42) x)", "42");

    test_transpile_and_run("multiple_vars", "(do (define-var x 10) (define-var y 20) (+ x y))",
                           "30");

    test_transpile_and_run("set_var", "(do (define-var x 10) (set x 42) x)", "42");
}

void test_if_statement() {
    test_transpile_and_run("if_then", "(if (< 5 10) 100 200)", "100");

    test_transpile_and_run("if_else", "(if (> 5 10) 100 200)", "200");

    test_transpile_and_run("nested_if", "(if (< 5 10) (if (= 3 3) 42 0) 99)", "42");
}

void test_while_loop() {
    test_transpile_and_run("while_simple",
                           R"((do
            (define-var counter 0)
            (define-var sum 0)
            (while (< counter 5)
                (do
                    (set sum (+ sum counter))
                    (set counter (+ counter 1))))
            sum))",
                           "10");
}

void test_for_loop() {
    test_transpile_and_run("for_simple",
                           R"((do
            (define-var total 0)
            (for (i 0 5)
                (set total (+ total i)))
            total))",
                           "10");
}

void test_functions() {
    test_transpile_and_run("simple_function",
                           R"((do
            (define-var (square x) (* x x))
            (square 7)))",
                           "49");

    test_transpile_and_run("two_param_function",
                           R"((do
            (define-var (add a b) (+ a b))
            (add 10 20)))",
                           "30");

    test_transpile_and_run("function_calling_function",
                           R"((do
            (define-func (double x) (* x 2))
            (define-func (quadruple x) (double (double x)))
            (quadruple 3)))",
                           "12");
}

void test_ffi() {
    test_transpile_and_run("ffi_abs", "(do (define-var x -42) (c++ \"std::abs(x)\"))", "42");
}

void test_bitwise() {
    test_transpile_and_run("bitwise_and", "(bit-and 12 10)", "8");
    test_transpile_and_run("bitwise_or", "(bit-or 12 10)", "14");
    test_transpile_and_run("bitwise_xor", "(bit-xor 12 10)", "6");
    test_transpile_and_run("bitwise_shl", "(bit-shl 5 2)", "20");
    test_transpile_and_run("bitwise_shr", "(bit-shr 20 2)", "5");
}

void test_structs() {
    // Basic struct definition and creation
    test_transpile_and_run("struct_basic",
                           "(do (define-struct token (type start end length)) (define-var tok "
                           "(make-token 1 0 5 5)) (token-type tok))",
                           "1");

    // Field access for all fields
    test_transpile_and_run(
        "struct_all_fields",
        "(do (define-struct token (type start end length)) (define-var tok (make-token 3 10 20 "
        "10)) (+ (token-type tok) (token-start tok) (token-end tok) (token-length tok)))",
        "43");

    // Field mutation
    test_transpile_and_run("struct_mutation",
                           "(do (define-struct token (type start end length)) (define-var tok "
                           "(make-token 1 0 5 5)) (set-token-type! tok 2) (token-type tok))",
                           "2");

    // Multiple mutations
    test_transpile_and_run("struct_multi_mutation",
                           "(do (define-struct token (type start end length)) (define-var tok "
                           "(make-token 1 0 5 5)) (set-token-type! tok 3) (set-token-length! tok "
                           "10) (+ (token-type tok) (token-length tok)))",
                           "13");

    // Struct fields in expressions
    test_transpile_and_run(
        "struct_in_expression",
        "(do (define-struct token (type start end length)) (define-var tok (make-token 1 5 15 10)) "
        "(if (= (token-type tok) 1) (+ (token-start tok) (token-length tok)) 0))",
        "15");

    // Multiple struct types
    test_transpile_and_run("multiple_struct_types",
                           "(do (define-struct point (x y)) (define-struct rect (left top right "
                           "bottom)) (define-var p (make-point 10 20)) (define-var r (make-rect 0 "
                           "0 100 100)) (+ (point-x p) (rect-right r)))",
                           "110");

    // Struct with single field
    test_transpile_and_run(
        "struct_single_field",
        "(do (define-struct wrapper (value)) (define-var w (make-wrapper 42)) (wrapper-value w))",
        "42");

    // Struct with many fields
    test_transpile_and_run("struct_many_fields",
                           "(do (define-struct data (f1 f2 f3 f4 f5 f6 f7 f8)) (define-var d "
                           "(make-data 1 2 3 4 5 6 7 8)) (+ (data-f1 d) (data-f8 d)))",
                           "9");
}

int main() {
    std::cout << "=== Lisp to C++ Transpiler Tests ===" << '\n';

    try {
        std::cout << "\n--- Simple Arithmetic ---" << '\n';
        test_simple_arithmetic();

        std::cout << "\n--- Multi-Argument Arithmetic ---" << '\n';
        test_multi_arg_arithmetic();

        std::cout << "\n--- Comparison ---" << '\n';
        test_comparison();

        std::cout << "\n--- Nested Expressions ---" << '\n';
        test_nested_expressions();

        std::cout << "\n--- Variables ---" << '\n';
        test_variables();

        std::cout << "\n--- If Statement ---" << '\n';
        test_if_statement();

        std::cout << "\n--- While Loop ---" << '\n';
        test_while_loop();

        std::cout << "\n--- For Loop ---" << '\n';
        test_for_loop();

        std::cout << "\n--- Functions ---" << '\n';
        test_functions();

        std::cout << "\n--- FFI (Native C++) ---" << '\n';
        test_ffi();

        std::cout << "\n--- Bitwise Operations ---" << '\n';
        test_bitwise();

        std::cout << "\n--- Structs ---" << '\n';
        test_structs();

        std::cout << "\n✓ All transpiler tests passed!" << '\n';
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Test failed: " << e.what() << '\n';
        return 1;
    }
}
