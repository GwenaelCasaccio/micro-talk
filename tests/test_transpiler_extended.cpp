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
    std::string cpp_file = "build/test_ext_" + name + ".cpp";
    std::string exe_file = "build/test_ext_" + name;
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

void test_string_literals() {
    test_transpile_and_run("string_literal", "\"hello\"", "hello");

    test_transpile_and_run("string_literal_spaces", "\"hello world\"", "hello world");
}

void test_string_operations() {
    test_transpile_and_run("string_length", "(string-length \"hello\")", "5");

    test_transpile_and_run("char_at", "(char-at \"hello\" 1)",
                           "101"); // ASCII 'e'

    test_transpile_and_run("substring", "(substring \"hello world\" 0 5)", "hello");

    test_transpile_and_run("string_concat", R"((string-concat "hello" " " "world"))",
                           "hello world");
}

void test_let_bindings() {
    test_transpile_and_run("let_simple", "(let ((x 10) (y 20)) (+ x y))", "30");

    test_transpile_and_run("let_nested", "(let ((x 10)) (let ((y 20)) (+ x y)))", "30");

    test_transpile_and_run("let_string", "(let ((s \"hello\")) (string-length s))", "5");

    test_transpile_and_run("let_shadowing", "(do (define-var x 100) (let ((x 42)) x))", "42");
}

void test_list_operations() {
    test_transpile_and_run("list_create", "(list-length (list 1 2 3 4 5))", "5");

    test_transpile_and_run("list_ref", "(list-ref (list 10 20 30 40) 2)", "30");

    test_transpile_and_run("list_set",
                           R"((do
            (define-var lst (list 1 2 3))
            (list-set! lst 1 99)
            (list-ref lst 1)))",
                           "99");
}

void test_string_parsing() {
    // Test case for parsing - count characters
    test_transpile_and_run("parse_count_chars",
                           R"((let ((text "hello"))
            (string-length text)))",
                           "5");

    // Test case for character iteration
    test_transpile_and_run("parse_iterate_chars",
                           R"((do
            (define-var text "abc")
            (define-var sum 0)
            (for (i 0 (string-length text))
                (set sum (+ sum (char-at text i))))
            sum))",
                           "294"); // 'a'=97 + 'b'=98 + 'c'=99
}

void test_tokenizer_example() {
    // Simple tokenizer - count spaces
    test_transpile_and_run("count_spaces",
                           R"((do
            (define-var text "hello world test")
            (define-var count 0)
            (for (i 0 (string-length text))
                (if (= (char-at text i) 32)
                    (set count (+ count 1))
                    0))
            count))",
                           "2");
}

int main() {
    std::cout << "=== Extended Transpiler Tests ===" << '\n';

    try {
        std::cout << "\n--- String Literals ---" << '\n';
        test_string_literals();

        std::cout << "\n--- String Operations ---" << '\n';
        test_string_operations();

        std::cout << "\n--- Let Bindings ---" << '\n';
        test_let_bindings();

        std::cout << "\n--- List Operations ---" << '\n';
        test_list_operations();

        std::cout << "\n--- String Parsing Examples ---" << '\n';
        test_string_parsing();

        std::cout << "\n--- Tokenizer Example ---" << '\n';
        test_tokenizer_example();

        std::cout << "\n✓ All extended transpiler tests passed!" << '\n';
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Test failed: " << e.what() << '\n';
        return 1;
    }
}
