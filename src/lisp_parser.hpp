#pragma once
#include <cctype>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <variant>
#include <vector>

// AST Node types
struct ASTNode;
using ASTNodePtr = std::shared_ptr<ASTNode>;

enum class NodeType { NUMBER, SYMBOL, LIST, STRING };

struct ASTNode {
    NodeType type;
    std::variant<int64_t, std::string, std::vector<ASTNodePtr>> value;

    static ASTNodePtr make_number(int64_t n) {
        auto node = std::make_shared<ASTNode>();
        node->type = NodeType::NUMBER;
        node->value = n;
        return node;
    }

    static ASTNodePtr make_symbol(const std::string& s) {
        auto node = std::make_shared<ASTNode>();
        node->type = NodeType::SYMBOL;
        node->value = s;
        return node;
    }

    static ASTNodePtr make_list(const std::vector<ASTNodePtr>& items) {
        auto node = std::make_shared<ASTNode>();
        node->type = NodeType::LIST;
        node->value = items;
        return node;
    }

    static ASTNodePtr make_string(const std::string& s) {
        auto node = std::make_shared<ASTNode>();
        node->type = NodeType::STRING;
        node->value = s;
        return node;
    }

    [[nodiscard]] int64_t as_number() const {
        return std::get<int64_t>(value);
    }

    [[nodiscard]] std::string as_symbol() const {
        return std::get<std::string>(value);
    }

    [[nodiscard]] std::string as_string() const {
        return std::get<std::string>(value);
    }

    [[nodiscard]] const std::vector<ASTNodePtr>& as_list() const {
        return std::get<std::vector<ASTNodePtr>>(value);
    }
};

class LispParser {
  private:
    std::string input;
    size_t pos{0};

    [[nodiscard]] char peek() const {
        return pos < input.size() ? input[pos] : '\0';
    }

    char next() {
        return pos < input.size() ? input[pos++] : '\0';
    }

    void skip_whitespace() {
        while (pos < input.size()) {
            // Skip whitespace
            if (std::isspace(input[pos]) != 0) {
                pos++;
            }
            // Skip comments (from ; to end of line)
            else if (input[pos] == ';') {
                while (pos < input.size() && input[pos] != '\n') {
                    pos++;
                }
                // Skip the newline itself
                if (pos < input.size() && input[pos] == '\n') {
                    pos++;
                }
            } else {
                break;
            }
        }
    }

    [[nodiscard]] static bool is_delimiter(char c) {
        return c == '(' || c == ')' || (std::isspace(c) != 0) || c == '\0';
    }

    ASTNodePtr parse_number() {
        std::string num_str;
        bool negative = false;

        if (peek() == '-') {
            negative = true;
            next();
        }

        while (std::isdigit(peek()) != 0) {
            num_str += next();
        }

        if (num_str.empty()) {
            throw std::runtime_error("Invalid number");
        }

        int64_t value = std::stoll(num_str);
        return ASTNode::make_number(negative ? -value : value);
    }

    ASTNodePtr parse_symbol() {
        std::string sym;

        while (!is_delimiter(peek())) {
            sym += next();
        }

        if (sym.empty()) {
            throw std::runtime_error("Invalid symbol");
        }

        return ASTNode::make_symbol(sym);
    }

    ASTNodePtr parse_string() {
        if (next() != '"') {
            throw std::runtime_error("Expected '\"'");
        }

        std::string str;

        while (peek() != '"' && peek() != '\0') {
            char c = next();
            if (c == '\\' && peek() != '\0') {
                // Handle escape sequences
                char escaped = next();
                switch (escaped) {
                    case 'n':
                        str += '\n';
                        break;
                    case 't':
                        str += '\t';
                        break;
                    case 'r':
                        str += '\r';
                        break;
                    case '\\':
                        str += '\\';
                        break;
                    case '"':
                        str += '"';
                        break;
                    default:
                        str += escaped;
                        break;
                }
            } else {
                str += c;
            }
        }

        if (next() != '"') {
            throw std::runtime_error("Expected closing '\"'");
        }

        return ASTNode::make_string(str);
    }

    ASTNodePtr parse_list() {
        if (next() != '(') {
            throw std::runtime_error("Expected '('");
        }

        std::vector<ASTNodePtr> items;
        skip_whitespace();

        while (peek() != ')' && peek() != '\0') {
            items.push_back(parse_expr());
            skip_whitespace();
        }

        if (next() != ')') {
            throw std::runtime_error("Expected ')'");
        }

        return ASTNode::make_list(items);
    }

    ASTNodePtr parse_expr() {
        skip_whitespace();

        char c = peek();

        if (c == '\0') {
            throw std::runtime_error("Unexpected end of input");
        }

        if (c == '(') {
            return parse_list();
        }

        if (c == '"') {
            return parse_string();
        }

        if ((std::isdigit(c) != 0) ||
            (c == '-' && pos + 1 < input.size() && (std::isdigit(input[pos + 1]) != 0))) {
            return parse_number();
        }

        return parse_symbol();
    }

  public:
    LispParser(std::string src) : input(std::move(src)) {}

    ASTNodePtr parse() {
        return parse_expr();
    }

    // Parse multiple top-level expressions
    std::vector<ASTNodePtr> parse_all() {
        std::vector<ASTNodePtr> exprs;
        skip_whitespace();

        while (peek() != '\0') {
            exprs.push_back(parse_expr());
            skip_whitespace();
        }

        return exprs;
    }
};
