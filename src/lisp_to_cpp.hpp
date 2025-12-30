#pragma once
#include "lisp_parser.hpp"
#include <sstream>
#include <map>
#include <set>
#include <stdexcept>

class LispToCppCompiler {
private:
    std::ostringstream code;
    std::ostringstream functions;
    std::map<std::string, std::string> variables;  // Lisp name -> C++ name
    std::set<std::string> function_names;
    int var_counter = 0;
    int label_counter = 0;
    int indent_level = 0;

    std::string get_indent() const {
        return std::string(indent_level * 4, ' ');
    }

    void increase_indent() { indent_level++; }
    void decrease_indent() { indent_level--; }

    std::string fresh_var() {
        return "var_" + std::to_string(var_counter++);
    }

    std::string fresh_label() {
        return "label_" + std::to_string(label_counter++);
    }

    std::string sanitize_name(const std::string& name) {
        std::string result;
        for (char c : name) {
            if (c == '-') result += '_';
            else if (c == '?' ) result += "_p";
            else if (c == '!') result += "_bang";
            else if (std::isalnum(c)) result += c;
            else result += '_';
        }
        return result;
    }

    std::string sanitize_function_name(const std::string& name) {
        // Prefix function names with "fn_" to avoid conflicts with C++ keywords
        return "fn_" + sanitize_name(name);
    }

    std::string compile_expr(const ASTNodePtr& node) {
        switch (node->type) {
            case NodeType::NUMBER:
                return std::to_string(node->as_number()) + "LL";

            case NodeType::STRING:
                // For now, strings are not supported in standalone mode
                throw std::runtime_error("String literals not yet supported in C++ transpiler");

            case NodeType::SYMBOL: {
                std::string sym = node->as_symbol();
                if (variables.find(sym) != variables.end()) {
                    return variables[sym];
                }
                throw std::runtime_error("Undefined variable: " + sym);
            }

            case NodeType::LIST: {
                const auto& list = node->as_list();
                if (list.empty()) {
                    throw std::runtime_error("Empty list in expression");
                }

                if (list[0]->type != NodeType::SYMBOL) {
                    throw std::runtime_error("First element of list must be a symbol");
                }

                std::string op = list[0]->as_symbol();

                // Arithmetic operations
                if (op == "+") return compile_binary_op(list, "+", true);
                if (op == "-") return compile_binary_op(list, "-", true);
                if (op == "*") return compile_binary_op(list, "*", true);
                if (op == "/") return compile_binary_op(list, "/", true);
                if (op == "%") return compile_binary_op(list, "%", true);

                // Comparison operations
                if (op == "=") return compile_binary_op(list, "==", false);
                if (op == "<") return compile_binary_op(list, "<", false);
                if (op == ">") return compile_binary_op(list, ">", false);
                if (op == "<=") return compile_binary_op(list, "<=", false);
                if (op == ">=") return compile_binary_op(list, ">=", false);

                // Bitwise operations
                if (op == "bit-and") return compile_binary_op(list, "&", false);
                if (op == "bit-or") return compile_binary_op(list, "|", false);
                if (op == "bit-xor") return compile_binary_op(list, "^", false);
                if (op == "bit-shl") return compile_binary_op(list, "<<", false);
                if (op == "bit-shr") return compile_binary_op(list, ">>", false);
                if (op == "bit-ashr") return compile_binary_op(list, ">>", false); // Same as >> for signed

                // Control flow
                if (op == "if") return compile_if(list);
                if (op == "while") return compile_while(list);
                if (op == "for") return compile_for(list);
                if (op == "do") return compile_do(list);

                // Variables
                if (op == "define") {
                    compile_define(list);
                    return "0LL"; // define returns 0
                }
                if (op == "set") {
                    compile_set(list);
                    return variables[list[1]->as_symbol()];
                }

                // FFI - Native C++ call
                if (op == "c++") {
                    if (list.size() != 2 || list[1]->type != NodeType::STRING) {
                        throw std::runtime_error("c++ requires a string argument: (c++ \"code\")");
                    }
                    return "(" + list[1]->as_string() + ")";
                }

                // Function call
                if (function_names.find(op) != function_names.end()) {
                    return compile_function_call(list);
                }

                throw std::runtime_error("Unknown operation: " + op);
            }
        }

        return "";
    }

    std::string compile_binary_op(const std::vector<ASTNodePtr>& list,
                                   const std::string& cpp_op,
                                   bool multi_arg) {
        if (multi_arg) {
            // Support multi-argument like (+ 1 2 3)
            if (list.size() < 2) {
                throw std::runtime_error("Binary operation requires at least 1 argument");
            }
            std::string result = "(" + compile_expr(list[1]);
            for (size_t i = 2; i < list.size(); i++) {
                result += " " + cpp_op + " " + compile_expr(list[i]);
            }
            result += ")";
            return result;
        } else {
            // Binary only
            if (list.size() != 3) {
                throw std::runtime_error("Binary operation requires exactly 2 arguments");
            }
            return "(" + compile_expr(list[1]) + " " + cpp_op + " " + compile_expr(list[2]) + ")";
        }
    }

    std::string compile_if(const std::vector<ASTNodePtr>& list) {
        if (list.size() != 4) {
            throw std::runtime_error("if requires 3 arguments: (if cond then else)");
        }

        std::string result_var = fresh_var();
        std::string cond = compile_expr(list[1]);

        code << get_indent() << "int64_t " << result_var << ";\n";
        code << get_indent() << "if (" << cond << ") {\n";
        increase_indent();
        std::string then_expr = compile_expr(list[2]);
        code << get_indent() << result_var << " = " << then_expr << ";\n";
        decrease_indent();
        code << get_indent() << "} else {\n";
        increase_indent();
        std::string else_expr = compile_expr(list[3]);
        code << get_indent() << result_var << " = " << else_expr << ";\n";
        decrease_indent();
        code << get_indent() << "}\n";

        return result_var;
    }

    std::string compile_while(const std::vector<ASTNodePtr>& list) {
        if (list.size() < 2) {
            throw std::runtime_error("while requires at least 1 argument: (while cond body...)");
        }

        std::string cond = compile_expr(list[1]);

        code << get_indent() << "while (" << cond << ") {\n";
        increase_indent();

        for (size_t i = 2; i < list.size(); i++) {
            std::string expr = compile_expr(list[i]);
            code << get_indent() << expr << ";\n";
        }

        // Re-evaluate condition
        cond = compile_expr(list[1]);

        decrease_indent();
        code << get_indent() << "}\n";

        return "0LL";
    }

    std::string compile_for(const std::vector<ASTNodePtr>& list) {
        if (list.size() < 3) {
            throw std::runtime_error("for requires at least 2 arguments: (for (var start end) body...)");
        }

        if (list[1]->type != NodeType::LIST || list[1]->as_list().size() != 3) {
            throw std::runtime_error("for loop spec must be (var start end)");
        }

        const auto& spec = list[1]->as_list();
        std::string loop_var = spec[0]->as_symbol();
        std::string cpp_var = sanitize_name(loop_var);

        std::string start = compile_expr(spec[1]);
        std::string end = compile_expr(spec[2]);

        // Save old variable if it exists
        std::string old_var;
        bool had_var = false;
        if (variables.find(loop_var) != variables.end()) {
            old_var = variables[loop_var];
            had_var = true;
        }

        variables[loop_var] = cpp_var;

        code << get_indent() << "for (int64_t " << cpp_var << " = " << start
             << "; " << cpp_var << " < " << end << "; " << cpp_var << "++) {\n";
        increase_indent();

        for (size_t i = 2; i < list.size(); i++) {
            std::string expr = compile_expr(list[i]);
            code << get_indent() << expr << ";\n";
        }

        decrease_indent();
        code << get_indent() << "}\n";

        // Restore old variable
        if (had_var) {
            variables[loop_var] = old_var;
        } else {
            variables.erase(loop_var);
        }

        return "0LL";
    }

    std::string compile_do(const std::vector<ASTNodePtr>& list) {
        if (list.size() < 2) {
            throw std::runtime_error("do requires at least 1 expression");
        }

        std::string result;
        for (size_t i = 1; i < list.size() - 1; i++) {
            std::string expr = compile_expr(list[i]);
            code << get_indent() << expr << ";\n";
        }

        // Return last expression
        result = compile_expr(list[list.size() - 1]);
        return result;
    }

    void compile_define(const std::vector<ASTNodePtr>& list) {
        if (list.size() < 3) {
            throw std::runtime_error("define requires at least 2 arguments");
        }

        // Check if it's a function definition: (define (name params...) body)
        if (list[1]->type == NodeType::LIST) {
            compile_function_def(list);
            return;
        }

        // Variable definition: (define var value)
        if (list[1]->type != NodeType::SYMBOL) {
            throw std::runtime_error("define requires a symbol as first argument");
        }

        std::string var_name = list[1]->as_symbol();
        std::string cpp_var = sanitize_name(var_name);

        std::string value = compile_expr(list[2]);

        // Check if variable already exists
        if (variables.find(var_name) == variables.end()) {
            code << get_indent() << "int64_t " << cpp_var << " = " << value << ";\n";
            variables[var_name] = cpp_var;
        } else {
            code << get_indent() << variables[var_name] << " = " << value << ";\n";
        }
    }

    void compile_set(const std::vector<ASTNodePtr>& list) {
        if (list.size() != 3) {
            throw std::runtime_error("set requires 2 arguments: (set var value)");
        }

        if (list[1]->type != NodeType::SYMBOL) {
            throw std::runtime_error("set requires a symbol as first argument");
        }

        std::string var_name = list[1]->as_symbol();

        if (variables.find(var_name) == variables.end()) {
            throw std::runtime_error("Cannot set undefined variable: " + var_name);
        }

        std::string value = compile_expr(list[2]);
        code << get_indent() << variables[var_name] << " = " << value << ";\n";
    }

    void compile_function_def(const std::vector<ASTNodePtr>& list) {
        // (define (name param1 param2...) body)
        const auto& sig = list[1]->as_list();
        if (sig.empty() || sig[0]->type != NodeType::SYMBOL) {
            throw std::runtime_error("Function definition requires a name");
        }

        std::string func_name = sig[0]->as_symbol();
        std::string cpp_func = sanitize_function_name(func_name);

        function_names.insert(func_name);

        // Build function signature
        std::ostringstream func_code;
        func_code << "int64_t " << cpp_func << "(";

        std::vector<std::string> param_names;
        for (size_t i = 1; i < sig.size(); i++) {
            if (sig[i]->type != NodeType::SYMBOL) {
                throw std::runtime_error("Function parameters must be symbols");
            }
            std::string param = sig[i]->as_symbol();
            std::string cpp_param = sanitize_name(param);
            param_names.push_back(param);

            if (i > 1) func_code << ", ";
            func_code << "int64_t " << cpp_param;
        }
        func_code << ") {\n";

        // Save current context
        auto old_variables = variables;
        auto old_code = code.str();
        code.str("");
        code.clear();
        indent_level = 1;

        // Add parameters to scope
        for (const auto& param : param_names) {
            variables[param] = sanitize_name(param);
        }

        // Compile function body
        std::string body_result = compile_expr(list[2]);
        func_code << get_indent() << "return " << body_result << ";\n";
        func_code << "}\n\n";

        // Restore context
        std::string func_body = code.str();
        code.str(old_code);
        code.clear();
        code << old_code;
        variables = old_variables;
        indent_level = 0;

        // Add function to functions section
        functions << func_code.str();
        // Include any code generated in function body
        if (!func_body.empty()) {
            functions << func_body;
        }
    }

    std::string compile_function_call(const std::vector<ASTNodePtr>& list) {
        std::string func_name = list[0]->as_symbol();
        std::string cpp_func = sanitize_function_name(func_name);

        std::ostringstream call;
        call << cpp_func << "(";
        for (size_t i = 1; i < list.size(); i++) {
            if (i > 1) call << ", ";
            call << compile_expr(list[i]);
        }
        call << ")";

        return call.str();
    }

public:
    std::string compile(const ASTNodePtr& ast) {
        // Reset state
        code.str("");
        code.clear();
        functions.str("");
        functions.clear();
        variables.clear();
        function_names.clear();
        var_counter = 0;
        label_counter = 0;
        indent_level = 1;

        // Start building the C++ program
        std::ostringstream program;
        program << "#include <cstdint>\n";
        program << "#include <iostream>\n\n";

        // Compile the main expression
        std::string result = compile_expr(ast);

        // Build complete program
        std::ostringstream full_code;
        full_code << program.str();

        // Add function definitions
        if (!functions.str().empty()) {
            full_code << "// Function definitions\n";
            full_code << functions.str();
        }

        full_code << "int main() {\n";
        full_code << code.str();
        full_code << "    int64_t result = " << result << ";\n";
        full_code << "    std::cout << result << std::endl;\n";
        full_code << "    return 0;\n";
        full_code << "}\n";

        return full_code.str();
    }
};
