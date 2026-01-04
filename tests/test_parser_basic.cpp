#include "../src/lisp_parser.hpp"
#include <cassert>
#include <iostream>

void test_parse_number() {
    std::cout << "Testing number parsing..." << std::endl;

    LispParser parser1("42");
    auto node1 = parser1.parse();
    assert(node1->type == NodeType::NUMBER);
    assert(node1->as_number() == 42);
    std::cout << "  ✓ Positive integer: 42" << std::endl;

    LispParser parser2("-17");
    auto node2 = parser2.parse();
    assert(node2->type == NodeType::NUMBER);
    assert(node2->as_number() == -17);
    std::cout << "  ✓ Negative integer: -17" << std::endl;

    LispParser parser3("0");
    auto node3 = parser3.parse();
    assert(node3->type == NodeType::NUMBER);
    assert(node3->as_number() == 0);
    std::cout << "  ✓ Zero: 0" << std::endl;

    LispParser parser4("999999");
    auto node4 = parser4.parse();
    assert(node4->type == NodeType::NUMBER);
    assert(node4->as_number() == 999999);
    std::cout << "  ✓ Large number: 999999" << std::endl;
}

void test_parse_symbol() {
    std::cout << "Testing symbol parsing..." << std::endl;

    LispParser parser1("foo");
    auto node1 = parser1.parse();
    assert(node1->type == NodeType::SYMBOL);
    assert(node1->as_symbol() == "foo");
    std::cout << "  ✓ Simple symbol: foo" << std::endl;

    LispParser parser2("+");
    auto node2 = parser2.parse();
    assert(node2->type == NodeType::SYMBOL);
    assert(node2->as_symbol() == "+");
    std::cout << "  ✓ Operator symbol: +" << std::endl;

    LispParser parser3("my-variable");
    auto node3 = parser3.parse();
    assert(node3->type == NodeType::SYMBOL);
    assert(node3->as_symbol() == "my-variable");
    std::cout << "  ✓ Hyphenated symbol: my-variable" << std::endl;

    LispParser parser4(">=");
    auto node4 = parser4.parse();
    assert(node4->type == NodeType::SYMBOL);
    assert(node4->as_symbol() == ">=");
    std::cout << "  ✓ Multi-char operator: >=" << std::endl;

    LispParser parser5("NULL");
    auto node5 = parser5.parse();
    assert(node5->type == NodeType::SYMBOL);
    assert(node5->as_symbol() == "NULL");
    std::cout << "  ✓ All-caps symbol: NULL" << std::endl;
}

void test_parse_string() {
    std::cout << "Testing string parsing..." << std::endl;

    LispParser parser1("\"hello\"");
    auto node1 = parser1.parse();
    assert(node1->type == NodeType::STRING);
    assert(node1->as_string() == "hello");
    std::cout << "  ✓ Simple string: \"hello\"" << std::endl;

    LispParser parser2("\"\"");
    auto node2 = parser2.parse();
    assert(node2->type == NodeType::STRING);
    assert(node2->as_string() == "");
    std::cout << "  ✓ Empty string: \"\"" << std::endl;

    LispParser parser3("\"hello world\"");
    auto node3 = parser3.parse();
    assert(node3->type == NodeType::STRING);
    assert(node3->as_string() == "hello world");
    std::cout << "  ✓ String with space: \"hello world\"" << std::endl;

    LispParser parser4("\"line1\\nline2\"");
    auto node4 = parser4.parse();
    assert(node4->type == NodeType::STRING);
    assert(node4->as_string() == "line1\nline2");
    std::cout << "  ✓ String with escape: \"line1\\nline2\"" << std::endl;

    LispParser parser5("\"quote: \\\"test\\\"\"");
    auto node5 = parser5.parse();
    assert(node5->type == NodeType::STRING);
    assert(node5->as_string() == "quote: \"test\"");
    std::cout << "  ✓ String with escaped quotes" << std::endl;
}

void test_parse_simple_list() {
    std::cout << "Testing simple list parsing..." << std::endl;

    LispParser parser1("(+ 1 2)");
    auto node1 = parser1.parse();
    assert(node1->type == NodeType::LIST);
    const auto& items1 = node1->as_list();
    assert(items1.size() == 3);
    assert(items1[0]->type == NodeType::SYMBOL);
    assert(items1[0]->as_symbol() == "+");
    assert(items1[1]->type == NodeType::NUMBER);
    assert(items1[1]->as_number() == 1);
    assert(items1[2]->type == NodeType::NUMBER);
    assert(items1[2]->as_number() == 2);
    std::cout << "  ✓ Simple arithmetic: (+ 1 2)" << std::endl;

    LispParser parser2("()");
    auto node2 = parser2.parse();
    assert(node2->type == NodeType::LIST);
    const auto& items2 = node2->as_list();
    assert(items2.size() == 0);
    std::cout << "  ✓ Empty list: ()" << std::endl;

    LispParser parser3("(foo bar baz)");
    auto node3 = parser3.parse();
    assert(node3->type == NodeType::LIST);
    const auto& items3 = node3->as_list();
    assert(items3.size() == 3);
    assert(items3[0]->as_symbol() == "foo");
    assert(items3[1]->as_symbol() == "bar");
    assert(items3[2]->as_symbol() == "baz");
    std::cout << "  ✓ Symbol list: (foo bar baz)" << std::endl;
}

void test_parse_nested_list() {
    std::cout << "Testing nested list parsing..." << std::endl;

    LispParser parser1("(+ (* 2 3) 4)");
    auto node1 = parser1.parse();
    assert(node1->type == NodeType::LIST);
    const auto& items1 = node1->as_list();
    assert(items1.size() == 3);
    assert(items1[0]->as_symbol() == "+");
    assert(items1[1]->type == NodeType::LIST);
    const auto& nested1 = items1[1]->as_list();
    assert(nested1.size() == 3);
    assert(nested1[0]->as_symbol() == "*");
    assert(nested1[1]->as_number() == 2);
    assert(nested1[2]->as_number() == 3);
    assert(items1[2]->as_number() == 4);
    std::cout << "  ✓ Nested expression: (+ (* 2 3) 4)" << std::endl;

    LispParser parser2("(if (< x 10) (+ x 1) (- x 1))");
    auto node2 = parser2.parse();
    assert(node2->type == NodeType::LIST);
    const auto& items2 = node2->as_list();
    assert(items2.size() == 4);
    assert(items2[0]->as_symbol() == "if");
    assert(items2[1]->type == NodeType::LIST);
    assert(items2[2]->type == NodeType::LIST);
    assert(items2[3]->type == NodeType::LIST);
    std::cout << "  ✓ Complex nested: (if (< x 10) (+ x 1) (- x 1))" << std::endl;
}

void test_parse_whitespace() {
    std::cout << "Testing whitespace handling..." << std::endl;

    LispParser parser1("   42   ");
    auto node1 = parser1.parse();
    assert(node1->as_number() == 42);
    std::cout << "  ✓ Leading/trailing whitespace" << std::endl;

    LispParser parser2("(  +   1    2  )");
    auto node2 = parser2.parse();
    const auto& items2 = node2->as_list();
    assert(items2.size() == 3);
    std::cout << "  ✓ Whitespace in list" << std::endl;

    LispParser parser3("(\n+\n1\n2\n)");
    auto node3 = parser3.parse();
    const auto& items3 = node3->as_list();
    assert(items3.size() == 3);
    std::cout << "  ✓ Newlines in list" << std::endl;

    LispParser parser4("(+ 1\t2)");
    auto node4 = parser4.parse();
    const auto& items4 = node4->as_list();
    assert(items4.size() == 3);
    std::cout << "  ✓ Tabs in list" << std::endl;
}

void test_parse_multiple_expressions() {
    std::cout << "Testing multiple expression parsing..." << std::endl;

    LispParser parser1("42 foo (+ 1 2)");
    auto exprs1 = parser1.parse_all();
    assert(exprs1.size() == 3);
    assert(exprs1[0]->type == NodeType::NUMBER);
    assert(exprs1[0]->as_number() == 42);
    assert(exprs1[1]->type == NodeType::SYMBOL);
    assert(exprs1[1]->as_symbol() == "foo");
    assert(exprs1[2]->type == NodeType::LIST);
    std::cout << "  ✓ Three expressions: 42 foo (+ 1 2)" << std::endl;

    LispParser parser2("(define-var x 10)\n(define-var y 20)\n(+ x y)");
    auto exprs2 = parser2.parse_all();
    assert(exprs2.size() == 3);
    std::cout << "  ✓ Multiple definitions with newlines" << std::endl;

    LispParser parser3("");
    auto exprs3 = parser3.parse_all();
    assert(exprs3.size() == 0);
    std::cout << "  ✓ Empty input returns empty vector" << std::endl;
}

int main() {
    std::cout << "=== Parser Basic Tests ===" << std::endl;

    try {
        test_parse_number();
        std::cout << std::endl;

        test_parse_symbol();
        std::cout << std::endl;

        test_parse_string();
        std::cout << std::endl;

        test_parse_simple_list();
        std::cout << std::endl;

        test_parse_nested_list();
        std::cout << std::endl;

        test_parse_whitespace();
        std::cout << std::endl;

        test_parse_multiple_expressions();

        std::cout << "\n✓ All parser basic tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Test failed: " << e.what() << std::endl;
        return 1;
    }
}
