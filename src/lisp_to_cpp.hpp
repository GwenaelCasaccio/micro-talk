#pragma once
#include "lisp_parser.hpp"
#include <sstream>
#include <map>
#include <set>
#include <algorithm>
#include <stdexcept>

class LispToCppCompiler {
private:
    struct StructDef {
        std::string name;
        std::vector<std::string> fields;
    };

    std::ostringstream code;
    std::ostringstream functions;
    std::ostringstream string_decls;  // String declarations at top of main
    std::ostringstream struct_decls;  // Struct declarations before functions
    std::map<std::string, std::string> variables;  // Lisp name -> C++ name
    std::map<std::string, std::string> variable_types;  // Track variable types (int64_t, string, vector, struct:name)
    std::map<std::string, StructDef> struct_defs;  // Struct name -> definition
    std::set<std::string> function_names;
    std::set<std::string> custom_includes;  // Additional C++ includes requested via (c++-include "header")
    int var_counter = 0;
    int label_counter = 0;
    int string_counter = 0;
    int struct_counter = 0;
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

    std::string fresh_string_var() {
        return "str_" + std::to_string(string_counter++);
    }

    std::string fresh_struct_var(const std::string& struct_name) {
        return sanitize_name(struct_name) + "_" + std::to_string(struct_counter++);
    }

    std::string struct_type_name(const std::string& struct_name) {
        // Convert "token" -> "Token" (capitalize first letter)
        std::string result = struct_name;
        if (!result.empty()) {
            result[0] = std::toupper(result[0]);
        }
        return result;
    }

    std::string escape_string(const std::string& str) {
        std::string result;
        for (char c : str) {
            if (c == '"') result += "\\\"";
            else if (c == '\\') result += "\\\\";
            else if (c == '\n') result += "\\n";
            else if (c == '\t') result += "\\t";
            else result += c;
        }
        return result;
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

            case NodeType::STRING: {
                // Create a string variable
                std::string str_var = fresh_string_var();
                string_decls << "    std::string " << str_var << " = \""
                            << escape_string(node->as_string()) << "\";\n";
                return str_var;
            }

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
                if (op == "let") return compile_let(list);

                // String operations
                if (op == "string-length") return compile_string_length(list);
                if (op == "char-at") return compile_char_at(list);
                if (op == "substring") return compile_substring(list);
                if (op == "string-concat") return compile_string_concat(list);

                // List/Array operations
                if (op == "list") return compile_list(list);
                if (op == "list-ref") return compile_list_ref(list);
                if (op == "list-length") return compile_list_length(list);
                if (op == "list-set!") return compile_list_set(list);

                // FFI - Native C++ call
                if (op == "c++") {
                    if (list.size() != 2 || list[1]->type != NodeType::STRING) {
                        throw std::runtime_error("c++ requires a string argument: (c++ \"code\")");
                    }
                    return "(" + list[1]->as_string() + ")";
                }

                // FFI - Add custom C++ include
                if (op == "c++-include") {
                    if (list.size() != 2 || list[1]->type != NodeType::STRING) {
                        throw std::runtime_error("c++-include requires a string argument: (c++-include \"header.h\")");
                    }
                    custom_includes.insert(list[1]->as_string());
                    return "0LL";
                }

                // Struct definition
                if (op == "define-struct") {
                    compile_struct_def(list);
                    return "0LL";
                }

                // Check for make-<structname> constructor
                if (op.substr(0, 5) == "make-") {
                    std::string struct_name = op.substr(5);
                    if (struct_defs.find(struct_name) != struct_defs.end()) {
                        return compile_make_struct(list, struct_name);
                    }
                }

                // Check for <structname>-<field> accessor or set-<structname>-<field>! mutator
                size_t dash_pos = op.find('-');
                if (dash_pos != std::string::npos) {
                    // Check for set-<structname>-<field>! mutator
                    if (op.substr(0, 4) == "set-" && op.back() == '!') {
                        std::string rest = op.substr(4, op.size() - 5);  // Remove "set-" and "!"
                        size_t rest_dash = rest.find('-');
                        if (rest_dash != std::string::npos) {
                            std::string potential_struct = rest.substr(0, rest_dash);
                            std::string potential_field = rest.substr(rest_dash + 1);

                            if (struct_defs.find(potential_struct) != struct_defs.end()) {
                                const auto& fields = struct_defs[potential_struct].fields;
                                if (std::find(fields.begin(), fields.end(), potential_field) != fields.end()) {
                                    return compile_struct_mutator(list, potential_struct, potential_field);
                                }
                            }
                        }
                    }

                    // Check for <structname>-<field> accessor
                    std::string potential_struct = op.substr(0, dash_pos);
                    std::string potential_field = op.substr(dash_pos + 1);

                    if (struct_defs.find(potential_struct) != struct_defs.end()) {
                        const auto& fields = struct_defs[potential_struct].fields;
                        if (std::find(fields.begin(), fields.end(), potential_field) != fields.end()) {
                            return compile_struct_accessor(list, potential_struct, potential_field);
                        }
                    }
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

        // Use a label-based loop that re-evaluates the condition each iteration
        std::string loop_label = fresh_label();
        std::string end_label = loop_label + "_end";

        code << get_indent() << loop_label << ":\n";
        increase_indent();

        // Evaluate condition at the start of each iteration
        std::string cond = compile_expr(list[1]);
        code << get_indent() << "if (!(" << cond << ")) goto " << end_label << ";\n";

        // Loop body
        for (size_t i = 2; i < list.size(); i++) {
            std::string expr = compile_expr(list[i]);
            code << get_indent() << expr << ";\n";
        }

        code << get_indent() << "goto " << loop_label << ";\n";

        decrease_indent();
        code << get_indent() << end_label << ":\n";

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
            // Determine type from value
            // Check if value is a known struct variable
            std::string value_type;
            for (const auto& vt : variable_types) {
                if (vt.first == value && vt.second.substr(0, 7) == "struct:") {
                    value_type = vt.second;
                    break;
                }
            }

            if (!value_type.empty() && value_type.substr(0, 7) == "struct:") {
                // It's a struct type
                std::string struct_name = value_type.substr(7);
                std::string cpp_struct = struct_type_name(struct_name);
                code << get_indent() << cpp_struct << " " << cpp_var << " = " << value << ";\n";
                variable_types[var_name] = value_type;
            } else if (value.find("str_") == 0 || value.find("std::string") != std::string::npos) {
                code << get_indent() << "std::string " << cpp_var << " = " << value << ";\n";
                variable_types[var_name] = "string";
            } else if (value.find("vec_") == 0 || value.find("std::vector") != std::string::npos) {
                code << get_indent() << "auto " << cpp_var << " = " << value << ";\n";
                variable_types[var_name] = "vector";
            } else {
                code << get_indent() << "int64_t " << cpp_var << " = " << value << ";\n";
                variable_types[var_name] = "int64_t";
            }
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

        // Get the generated code from the function body
        std::string func_body = code.str();

        // Insert the function body code before the return statement
        if (!func_body.empty()) {
            func_code << func_body;
        }
        func_code << get_indent() << "return " << body_result << ";\n";
        func_code << "}\n\n";

        // Restore context
        code.str(old_code);
        code.clear();
        code << old_code;
        variables = old_variables;
        indent_level = 0;

        // Add function to functions section
        functions << func_code.str();
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

    // Let bindings: (let ((var1 val1) (var2 val2)) body...)
    std::string compile_let(const std::vector<ASTNodePtr>& list) {
        if (list.size() < 3) {
            throw std::runtime_error("let requires at least 2 arguments: (let ((bindings...)) body...)");
        }

        if (list[1]->type != NodeType::LIST) {
            throw std::runtime_error("let bindings must be a list");
        }

        const auto& bindings = list[1]->as_list();

        // Save current variable scope
        auto old_variables = variables;
        auto old_var_types = variable_types;

        // Declare result variable BEFORE the block (we'll determine type later)
        std::string result_var = fresh_var();
        // We'll declare it as int64_t by default, but might need to change this
        code << get_indent() << "int64_t " << result_var << " = 0LL;\n";

        // Create a block for the let scope
        code << get_indent() << "{\n";
        increase_indent();

        // Process bindings
        for (const auto& binding : bindings) {
            if (binding->type != NodeType::LIST || binding->as_list().size() != 2) {
                throw std::runtime_error("Each let binding must be (var value)");
            }

            const auto& bind_list = binding->as_list();
            if (bind_list[0]->type != NodeType::SYMBOL) {
                throw std::runtime_error("Let binding variable must be a symbol");
            }

            std::string var_name = bind_list[0]->as_symbol();
            std::string cpp_var = sanitize_name(var_name);
            std::string value = compile_expr(bind_list[1]);

            // Determine type from value - simple heuristic
            if (value.find("str_") == 0 || value.find("std::string") != std::string::npos) {
                code << get_indent() << "std::string " << cpp_var << " = " << value << ";\n";
                variable_types[var_name] = "string";
            } else if (value.find("vec_") == 0 || value.find("std::vector") != std::string::npos) {
                code << get_indent() << "auto " << cpp_var << " = " << value << ";\n";
                variable_types[var_name] = "vector";
            } else {
                code << get_indent() << "int64_t " << cpp_var << " = " << value << ";\n";
                variable_types[var_name] = "int64_t";
            }

            variables[var_name] = cpp_var;
        }

        // Compile body expressions
        std::string result;
        for (size_t i = 2; i < list.size(); i++) {
            if (i < list.size() - 1) {
                std::string expr = compile_expr(list[i]);
                code << get_indent() << expr << ";\n";
            } else {
                result = compile_expr(list[i]);
            }
        }

        // Store result - just assign, already declared above
        code << get_indent() << result_var << " = " << result << ";\n";

        decrease_indent();
        code << get_indent() << "}\n";

        // Restore variable scope
        variables = old_variables;
        variable_types = old_var_types;

        return result_var;
    }

    // String operations
    std::string compile_string_length(const std::vector<ASTNodePtr>& list) {
        if (list.size() != 2) {
            throw std::runtime_error("string-length requires 1 argument");
        }
        std::string str = compile_expr(list[1]);
        return "(int64_t)" + str + ".length()";
    }

    std::string compile_char_at(const std::vector<ASTNodePtr>& list) {
        if (list.size() != 3) {
            throw std::runtime_error("char-at requires 2 arguments: (char-at string index)");
        }
        std::string str = compile_expr(list[1]);
        std::string index = compile_expr(list[2]);
        return "(int64_t)" + str + "[" + index + "]";
    }

    std::string compile_substring(const std::vector<ASTNodePtr>& list) {
        if (list.size() != 4) {
            throw std::runtime_error("substring requires 3 arguments: (substring string start end)");
        }
        std::string str = compile_expr(list[1]);
        std::string start = compile_expr(list[2]);
        std::string end = compile_expr(list[3]);

        std::string result_var = fresh_string_var();
        string_decls << "    std::string " << result_var << ";\n";
        code << get_indent() << result_var << " = " << str
             << ".substr(" << start << ", " << end << " - " << start << ");\n";
        return result_var;
    }

    std::string compile_string_concat(const std::vector<ASTNodePtr>& list) {
        if (list.size() < 2) {
            throw std::runtime_error("string-concat requires at least 1 argument");
        }

        std::string result_var = fresh_string_var();
        string_decls << "    std::string " << result_var << ";\n";

        code << get_indent() << result_var << " = ";
        for (size_t i = 1; i < list.size(); i++) {
            if (i > 1) code << " + ";
            code << compile_expr(list[i]);
        }
        code << ";\n";

        return result_var;
    }

    // List/Array operations (using std::vector<int64_t>)
    std::string compile_list(const std::vector<ASTNodePtr>& list) {
        std::string vec_var = "vec_" + std::to_string(var_counter++);

        code << get_indent() << "std::vector<int64_t> " << vec_var << " = {";
        for (size_t i = 1; i < list.size(); i++) {
            if (i > 1) code << ", ";
            code << compile_expr(list[i]);
        }
        code << "};\n";

        return vec_var;
    }

    std::string compile_list_ref(const std::vector<ASTNodePtr>& list) {
        if (list.size() != 3) {
            throw std::runtime_error("list-ref requires 2 arguments: (list-ref list index)");
        }
        std::string vec = compile_expr(list[1]);
        std::string index = compile_expr(list[2]);
        return vec + "[" + index + "]";
    }

    std::string compile_list_length(const std::vector<ASTNodePtr>& list) {
        if (list.size() != 2) {
            throw std::runtime_error("list-length requires 1 argument");
        }
        std::string vec = compile_expr(list[1]);
        return "(int64_t)" + vec + ".size()";
    }

    std::string compile_list_set(const std::vector<ASTNodePtr>& list) {
        if (list.size() != 4) {
            throw std::runtime_error("list-set! requires 3 arguments: (list-set! list index value)");
        }
        std::string vec = compile_expr(list[1]);
        std::string index = compile_expr(list[2]);
        std::string value = compile_expr(list[3]);

        code << get_indent() << vec << "[" << index << "] = " << value << ";\n";
        return value;
    }

    // Struct operations
    void compile_struct_def(const std::vector<ASTNodePtr>& list) {
        // (define-struct token (type start end length))
        if (list.size() != 3) {
            throw std::runtime_error("define-struct requires 2 arguments: (define-struct name (fields...))");
        }

        if (list[1]->type != NodeType::SYMBOL) {
            throw std::runtime_error("Struct name must be a symbol");
        }

        if (list[2]->type != NodeType::LIST) {
            throw std::runtime_error("Struct fields must be a list");
        }

        std::string struct_name = list[1]->as_symbol();
        const auto& field_nodes = list[2]->as_list();

        StructDef def;
        def.name = struct_name;

        for (const auto& field : field_nodes) {
            if (field->type != NodeType::SYMBOL) {
                throw std::runtime_error("Struct fields must be symbols");
            }
            def.fields.push_back(field->as_symbol());
        }

        struct_defs[struct_name] = def;

        // Generate C++ struct definition
        std::string cpp_struct = struct_type_name(struct_name);
        struct_decls << "struct " << cpp_struct << " {\n";

        // Fields
        for (const auto& field : def.fields) {
            struct_decls << "    int64_t " << field << ";\n";
        }
        struct_decls << "\n";

        // Default constructor
        struct_decls << "    " << cpp_struct << "() : ";
        for (size_t i = 0; i < def.fields.size(); i++) {
            if (i > 0) struct_decls << ", ";
            struct_decls << def.fields[i] << "(0)";
        }
        struct_decls << " {}\n\n";

        // Parameterized constructor
        struct_decls << "    " << cpp_struct << "(";
        for (size_t i = 0; i < def.fields.size(); i++) {
            if (i > 0) struct_decls << ", ";
            struct_decls << "int64_t " << def.fields[i] << "_";
        }
        struct_decls << ")\n        : ";
        for (size_t i = 0; i < def.fields.size(); i++) {
            if (i > 0) struct_decls << ", ";
            struct_decls << def.fields[i] << "(" << def.fields[i] << "_)";
        }
        struct_decls << " {}\n";

        struct_decls << "};\n\n";
    }

    std::string compile_make_struct(const std::vector<ASTNodePtr>& list, const std::string& struct_name) {
        // (make-token 1 0 5 5)
        const auto& def = struct_defs[struct_name];

        if (list.size() != def.fields.size() + 1) {
            throw std::runtime_error("make-" + struct_name + " requires " +
                                   std::to_string(def.fields.size()) + " arguments");
        }

        std::string var_name = fresh_struct_var(struct_name);
        std::string cpp_struct = struct_type_name(struct_name);

        code << get_indent() << cpp_struct << " " << var_name << "(";
        for (size_t i = 1; i < list.size(); i++) {
            if (i > 1) code << ", ";
            code << compile_expr(list[i]);
        }
        code << ");\n";

        // Track as struct type
        variable_types[var_name] = "struct:" + struct_name;

        return var_name;
    }

    std::string compile_struct_accessor(const std::vector<ASTNodePtr>& list,
                                       const std::string& struct_name,
                                       const std::string& field) {
        // (token-type tok)
        if (list.size() != 2) {
            throw std::runtime_error(struct_name + "-" + field + " requires 1 argument");
        }

        std::string obj = compile_expr(list[1]);
        return obj + "." + field;
    }

    std::string compile_struct_mutator(const std::vector<ASTNodePtr>& list,
                                      const std::string& struct_name,
                                      const std::string& field) {
        // (set-token-type! tok 2)
        if (list.size() != 3) {
            throw std::runtime_error("set-" + struct_name + "-" + field + "! requires 2 arguments");
        }

        std::string obj = compile_expr(list[1]);
        std::string value = compile_expr(list[2]);

        code << get_indent() << obj << "." << field << " = " << value << ";\n";
        return value;
    }

public:
    std::string compile(const ASTNodePtr& ast) {
        // Reset state
        code.str("");
        code.clear();
        functions.str("");
        functions.clear();
        string_decls.str("");
        string_decls.clear();
        struct_decls.str("");
        struct_decls.clear();
        variables.clear();
        variable_types.clear();
        struct_defs.clear();
        function_names.clear();
        custom_includes.clear();
        var_counter = 0;
        label_counter = 0;
        string_counter = 0;
        struct_counter = 0;
        indent_level = 1;

        // Start building the C++ program
        std::ostringstream program;
        program << "#include <cstdint>\n";
        program << "#include <iostream>\n";
        program << "#include <iomanip>\n";
        program << "#include <fstream>\n";
        program << "#include <sstream>\n";
        program << "#include <string>\n";
        program << "#include <vector>\n";

        // Add custom includes
        for (const auto& inc : custom_includes) {
            program << "#include <" << inc << ">\n";
        }
        program << "\n";

        // Compile the main expression
        std::string result = compile_expr(ast);

        // Build complete program
        std::ostringstream full_code;
        full_code << program.str();

        // Add struct definitions BEFORE function definitions
        if (!struct_decls.str().empty()) {
            full_code << "// Struct definitions\n";
            full_code << struct_decls.str();
            full_code << "\n";
        }

        // Add function definitions
        if (!functions.str().empty()) {
            full_code << "// Function definitions\n";
            full_code << functions.str();
        }

        full_code << "int main(int argc, char** argv) {\n";

        // Add string declarations first
        if (!string_decls.str().empty()) {
            full_code << string_decls.str();
            full_code << "\n";
        }

        full_code << code.str();

        // Handle different result types
        // Check if it's a pure vector variable (no indexing or operations)
        bool is_pure_vec = (result.find("vec_") == 0 && result.find('[') == std::string::npos
                           && result.find('(') == std::string::npos);
        bool is_pure_str = (result.find("str_") == 0 && result.find('[') == std::string::npos
                           && result.find('(') == std::string::npos);

        if (is_pure_str) {
            // String result
            full_code << "    std::cout << " << result << " << std::endl;\n";
        } else if (is_pure_vec) {
            // Vector result - print size
            full_code << "    std::cout << " << result << ".size() << std::endl;\n";
        } else {
            // Integer result (or expression)
            full_code << "    int64_t result = " << result << ";\n";
            full_code << "    std::cout << result << std::endl;\n";
        }

        full_code << "    return 0;\n";
        full_code << "}\n";

        return full_code.str();
    }
};
