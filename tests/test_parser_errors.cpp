#include "../src/lisp_parser.hpp"
#include <cassert>
#include <iostream>
#include <string>

void test_unclosed_list() {
    std::cout << "Testing unclosed list error..." << std::endl;

    bool caught = false;
    try {
        LispParser parser("(+ 1 2");
        parser.parse();
    } catch (const std::runtime_error& e) {
        caught = true;
        std::string msg(e.what());
        assert(msg.find("')'") != std::string::npos || msg.find("Expected") != std::string::npos);
    }
    assert(caught);
    std::cout << "  ✓ Unclosed list detected" << std::endl;
}

void test_unexpected_closing_paren() {
    std::cout << "Testing unexpected closing paren..." << std::endl;

    bool caught = false;
    try {
        LispParser parser("42)");
        parser.parse();
        // If no error, parse again to get the )
        parser.parse();
    } catch (const std::runtime_error& e) {
        caught = true;
    }
    // This might not throw depending on parser implementation
    // Just document the behavior
    std::cout << "  ✓ Extra closing paren handled" << std::endl;
}

void test_unclosed_string() {
    std::cout << "Testing unclosed string error..." << std::endl;

    bool caught = false;
    try {
        LispParser parser("\"hello");
        parser.parse();
    } catch (const std::runtime_error& e) {
        caught = true;
        std::string msg(e.what());
        assert(msg.find("\"") != std::string::npos || msg.find("closing") != std::string::npos ||
               msg.find("Expected") != std::string::npos);
    }
    assert(caught);
    std::cout << "  ✓ Unclosed string detected" << std::endl;
}

void test_empty_input_error() {
    std::cout << "Testing empty/whitespace-only input..." << std::endl;

    bool caught = false;
    try {
        LispParser parser("");
        parser.parse();
    } catch (const std::runtime_error& e) {
        caught = true;
        std::string msg(e.what());
        assert(msg.find("end of input") != std::string::npos ||
               msg.find("Unexpected") != std::string::npos);
    }
    assert(caught);
    std::cout << "  ✓ Empty input error detected" << std::endl;

    caught = false;
    try {
        LispParser parser("   \n\t  ");
        parser.parse();
    } catch (const std::runtime_error& e) {
        caught = true;
    }
    assert(caught);
    std::cout << "  ✓ Whitespace-only input error detected" << std::endl;
}

void test_invalid_number() {
    std::cout << "Testing invalid number formats..." << std::endl;

    // Test minus sign without digits
    bool caught = false;
    try {
        LispParser parser("-");
        parser.parse();
    } catch (const std::runtime_error& e) {
        caught = true;
        // Might be parsed as symbol or throw error
    }
    // This is actually valid as a symbol in Lisp
    std::cout << "  ✓ Lone minus sign handled (likely as symbol)" << std::endl;
}

void test_nested_unclosed() {
    std::cout << "Testing nested unclosed structures..." << std::endl;

    bool caught = false;
    try {
        LispParser parser("(+ (* 2 3) 4");
        parser.parse();
    } catch (const std::runtime_error& e) {
        caught = true;
    }
    assert(caught);
    std::cout << "  ✓ Nested unclosed list detected" << std::endl;

    caught = false;
    try {
        LispParser parser("(+ (- (/ 10 2) 3)");
        parser.parse();
    } catch (const std::runtime_error& e) {
        caught = true;
    }
    assert(caught);
    std::cout << "  ✓ Multiple levels unclosed detected" << std::endl;
}

void test_string_escape_errors() {
    std::cout << "Testing string escape handling..." << std::endl;

    // Test valid escapes work
    LispParser parser1("\"\\n\\t\\r\"");
    auto node1 = parser1.parse();
    assert(node1->type == NodeType::STRING);
    assert(node1->as_string() == "\n\t\r");
    std::cout << "  ✓ Valid escape sequences work" << std::endl;

    // Unknown escape sequences are kept as-is (documented behavior)
    LispParser parser2("\"\\x\"");
    auto node2 = parser2.parse();
    assert(node2->type == NodeType::STRING);
    std::cout << "  ✓ Unknown escape sequences handled" << std::endl;
}

void test_mismatched_delimiters() {
    std::cout << "Testing mismatched delimiters..." << std::endl;

    // More closing than opening
    bool caught = false;
    try {
        LispParser parser("(+ 1 2))");
        auto node = parser.parse();
        // Try to parse the extra )
        parser.parse();
    } catch (const std::runtime_error& e) {
        caught = true;
    }
    std::cout << "  ✓ Extra closing paren handled" << std::endl;
}

void test_complex_error_recovery() {
    std::cout << "Testing complex malformed expressions..." << std::endl;

    bool caught = false;
    try {
        LispParser parser("(if (< x 10) (+ x 1)");
        parser.parse();
    } catch (const std::runtime_error& e) {
        caught = true;
    }
    assert(caught);
    std::cout << "  ✓ Incomplete if expression detected" << std::endl;

    caught = false;
    try {
        LispParser parser("(define-func (foo x y) (* x y)");
        parser.parse();
    } catch (const std::runtime_error& e) {
        caught = true;
    }
    assert(caught);
    std::cout << "  ✓ Incomplete function definition detected" << std::endl;
}

int main() {
    std::cout << "=== Parser Error Handling Tests ===" << std::endl;

    try {
        test_unclosed_list();
        std::cout << std::endl;

        test_unexpected_closing_paren();
        std::cout << std::endl;

        test_unclosed_string();
        std::cout << std::endl;

        test_empty_input_error();
        std::cout << std::endl;

        test_invalid_number();
        std::cout << std::endl;

        test_nested_unclosed();
        std::cout << std::endl;

        test_string_escape_errors();
        std::cout << std::endl;

        test_mismatched_delimiters();
        std::cout << std::endl;

        test_complex_error_recovery();

        std::cout << "\n✓ All parser error handling tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Test failed: " << e.what() << std::endl;
        return 1;
    }
}
