#include "../src/lisp_parser.hpp"
#include <iostream>
#include <cassert>

void test_single_line_comment() {
    std::cout << "Testing single line comments..." << std::endl;

    LispParser parser1("; This is a comment\n42");
    auto node1 = parser1.parse();
    assert(node1->type == NodeType::NUMBER);
    assert(node1->as_number() == 42);
    std::cout << "  ✓ Comment before expression" << std::endl;

    LispParser parser2("42 ; inline comment");
    auto node2 = parser2.parse();
    assert(node2->type == NodeType::NUMBER);
    assert(node2->as_number() == 42);
    std::cout << "  ✓ Inline comment after expression" << std::endl;

    LispParser parser3("; comment1\n; comment2\n42");
    auto node3 = parser3.parse();
    assert(node3->as_number() == 42);
    std::cout << "  ✓ Multiple consecutive comments" << std::endl;
}

void test_comments_in_lists() {
    std::cout << "Testing comments in lists..." << std::endl;

    LispParser parser1("(+ ; add operator\n 1 2)");
    auto node1 = parser1.parse();
    assert(node1->type == NodeType::LIST);
    const auto& items1 = node1->as_list();
    assert(items1.size() == 3);
    assert(items1[0]->as_symbol() == "+");
    assert(items1[1]->as_number() == 1);
    assert(items1[2]->as_number() == 2);
    std::cout << "  ✓ Comment inside list" << std::endl;

    LispParser parser2("(\n; operator\n+\n; first arg\n1\n; second arg\n2\n)");
    auto node2 = parser2.parse();
    const auto& items2 = node2->as_list();
    assert(items2.size() == 3);
    std::cout << "  ✓ Comments between all elements" << std::endl;

    LispParser parser3("(do\n  ; Step 1\n  (define x 10)\n  ; Step 2\n  (define y 20)\n  ; Result\n  (+ x y))");
    auto node3 = parser3.parse();
    const auto& items3 = node3->as_list();
    assert(items3[0]->as_symbol() == "do");
    assert(items3.size() == 4);
    std::cout << "  ✓ Comments in do block" << std::endl;
}

void test_comments_with_multiple_expressions() {
    std::cout << "Testing comments with multiple expressions..." << std::endl;

    std::string code = R"(
        ; Define x
        (define x 10)

        ; Define y
        (define y 20)

        ; Calculate sum
        (+ x y)
    )";

    LispParser parser(code);
    auto exprs = parser.parse_all();
    assert(exprs.size() == 3);
    std::cout << "  ✓ Comments between multiple expressions" << std::endl;
}

void test_comment_edge_cases() {
    std::cout << "Testing comment edge cases..." << std::endl;

    LispParser parser1(";");
    auto exprs1 = parser1.parse_all();
    assert(exprs1.size() == 0);
    std::cout << "  ✓ Comment only (no content)" << std::endl;

    LispParser parser2(";\n42");
    auto node2 = parser2.parse();
    assert(node2->as_number() == 42);
    std::cout << "  ✓ Empty comment line" << std::endl;

    LispParser parser3("; comment at end\n");
    auto exprs3 = parser3.parse_all();
    assert(exprs3.size() == 0);
    std::cout << "  ✓ Comment at end of file" << std::endl;

    LispParser parser4("42 ;comment");
    auto node4 = parser4.parse();
    // Semicolon starts a comment even without newline at end
    assert(node4->type == NodeType::NUMBER);
    assert(node4->as_number() == 42);
    std::cout << "  ✓ Comment without newline at end works" << std::endl;

    LispParser parser5("42 ;");
    auto node5 = parser5.parse();
    assert(node5->as_number() == 42);
    std::cout << "  ✓ Comment with no text after semicolon" << std::endl;
}

void test_comments_dont_affect_strings() {
    std::cout << "Testing comments don't affect strings..." << std::endl;

    LispParser parser1("\"this ; is not a comment\"");
    auto node1 = parser1.parse();
    assert(node1->type == NodeType::STRING);
    assert(node1->as_string() == "this ; is not a comment");
    std::cout << "  ✓ Semicolon inside string is not a comment" << std::endl;

    LispParser parser2("(print \"value: ; still a string\")");
    auto node2 = parser2.parse();
    const auto& items = node2->as_list();
    assert(items[1]->as_string() == "value: ; still a string");
    std::cout << "  ✓ Semicolon in string within list" << std::endl;
}

void test_nested_with_comments() {
    std::cout << "Testing deeply nested structures with comments..." << std::endl;

    std::string code = R"(
        (if ; condition check
            (< x 10) ; x less than 10?
            ; then branch
            (do
                ; increment x
                (+ x 1)
            )
            ; else branch
            (do
                ; decrement x
                (- x 1)
            )
        )
    )";

    LispParser parser(code);
    auto node = parser.parse();
    assert(node->type == NodeType::LIST);
    const auto& items = node->as_list();
    assert(items[0]->as_symbol() == "if");
    assert(items.size() == 4);
    std::cout << "  ✓ Comments in deeply nested if expression" << std::endl;
}

int main() {
    std::cout << "=== Parser Comment Tests ===" << std::endl;

    try {
        test_single_line_comment();
        std::cout << std::endl;

        test_comments_in_lists();
        std::cout << std::endl;

        test_comments_with_multiple_expressions();
        std::cout << std::endl;

        test_comment_edge_cases();
        std::cout << std::endl;

        test_comments_dont_affect_strings();
        std::cout << std::endl;

        test_nested_with_comments();

        std::cout << "\n✓ All parser comment tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Test failed: " << e.what() << std::endl;
        return 1;
    }
}
