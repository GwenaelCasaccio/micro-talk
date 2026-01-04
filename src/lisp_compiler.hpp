#pragma once
#include "lisp_parser.hpp"
#include "stack_vm.hpp"
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

class LispCompiler {
  private:
    std::vector<uint64_t> bytecode;
    std::map<std::string, uint64_t> labels;
    std::vector<std::pair<size_t, std::string>> label_refs;

    struct Variable {
        bool is_global;
        uint64_t addr;
    };

    // Environment management - stack of scopes
    struct Scope {
        std::map<std::string, Variable> variables;
    };

    bool is_in_function;
    std::vector<Scope> scopes;
    uint64_t next_var_address;
    uint64_t function_local_var_index;
    uint64_t function_temporary_var_index;

    // Function definitions
    struct Function {
        std::vector<std::string> params;
        ASTNodePtr body;
        uint64_t code_address;
    };
    std::map<std::string, Function> functions;

    static constexpr uint64_t VAR_START = 16384;

    void emit(uint64_t value) {
        bytecode.push_back(value);
    }

    void emit_opcode(Opcode op) {
        emit(static_cast<uint64_t>(op));
    }

    size_t current_address() const {
        return bytecode.size();
    }

    // Push a new scope
    void push_scope() {
        scopes.push_back(Scope());
    }

    // Pop the current scope
    void pop_scope() {
        if (scopes.empty()) {
            throw std::runtime_error("Cannot pop empty scope");
        }

        scopes.pop_back();
    }

    // Look up variable in current and parent scopes
    std::optional<Variable> lookup_variable(const std::string& name) {
        // Search from innermost to outermost scope
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            auto var_it = it->variables.find(name);
            if (var_it != it->variables.end()) {
                return var_it->second;
            }
        }
        return {};
    }

    // Define global variable in current scope
    uint64_t define_variable(const std::string& name) {
        if (scopes.empty()) {
            throw std::runtime_error("No active scope");
        }

        auto& current_scope = scopes.back();
        if (current_scope.variables.find(name) != current_scope.variables.end()) {
            throw std::runtime_error("Variable already defined in current scope: " + name);
        }

        uint64_t addr = next_var_address++;
        current_scope.variables[name] = {.is_global = true, .addr = addr};
        return addr;
    }

    uint64_t define_argument_variable(const std::string& name) {
        if (scopes.empty()) {
            throw std::runtime_error("No active scope");
        }

        auto& current_scope = scopes.back();
        if (current_scope.variables.find(name) != current_scope.variables.end()) {
            throw std::runtime_error("Variable already defined in current scope: " + name);
        }

        uint64_t addr = function_local_var_index++;
        current_scope.variables[name] = {.is_global = false, .addr = addr};
        return addr;
    }

    uint64_t define_temporary_variable(const std::string& name) {
        if (scopes.empty()) {
            throw std::runtime_error("No active scope");
        }

        auto& current_scope = scopes.back();
        if (current_scope.variables.find(name) != current_scope.variables.end()) {
            throw std::runtime_error("Variable already defined in current scope: " + name);
        }

        uint64_t addr = function_temporary_var_index++;
        current_scope.variables[name] = {.is_global = false, .addr = addr};
        return addr;
    }

    void compile_expr(const ASTNodePtr& node) {
        switch (node->type) {
            case NodeType::NUMBER:
                emit_opcode(Opcode::PUSH);
                emit(static_cast<uint64_t>(node->as_number()));
                break;

            case NodeType::STRING: {
                // Compile string literal to memory allocation
                std::string str = node->as_string();
                compile_string_literal(str);
                break;
            }

            case NodeType::SYMBOL: {
                std::string sym = node->as_symbol();

                std::optional<Variable> addr = lookup_variable(sym);
                if (addr) {
                    emit_opcode(Opcode::PUSH);
                    emit(addr->addr);
                    if (addr->is_global) {
                        emit_opcode(Opcode::LOAD);
                    } else {
                        emit_opcode(Opcode::BP_LOAD);
                    }
                } else {
                    if (functions.find(sym) != functions.end()) {
                        throw std::runtime_error("Cannot use function as value: " + sym);
                    }

                    throw std::runtime_error("Unbound symbol: " + sym);
                }
                break;
            }

            case NodeType::LIST: {
                const auto& items = node->as_list();

                if (items.empty()) {
                    throw std::runtime_error("Empty list not allowed");
                }

                if (items[0]->type != NodeType::SYMBOL) {
                    throw std::runtime_error("First element of list must be a symbol");
                }

                std::string op = items[0]->as_symbol();

                // Check if it's a function call
                if (functions.find(op) != functions.end()) {
                    compile_function_call(op, items);
                    return;
                }

                // Variable binding forms
                if (op == "define-func") {
                    compile_define_function(items);
                } else if (op == "define-var") {
                    compile_define_variable(items);
                } else if (op == "set") {
                    compile_set(items);
                } else if (op == "let") {
                    compile_let(items);
                }
                // Loop constructs
                else if (op == "while") {
                    compile_while(items);
                } else if (op == "for") {
                    compile_for(items);
                }
                // Memory primitives
                else if (op == "peek") {
                    compile_peek(items);
                } else if (op == "poke") {
                    compile_poke(items);
                }
                // Basic arithmetic operators
                else if (op == "+") {
                    if (items.size() < 3)
                        throw std::runtime_error("+ requires at least 2 arguments");
                    compile_expr(items[1]);
                    for (size_t i = 2; i < items.size(); i++) {
                        compile_expr(items[i]);
                        emit_opcode(Opcode::ADD);
                    }
                } else if (op == "-") {
                    if (items.size() < 3)
                        throw std::runtime_error("- requires at least 2 arguments");
                    compile_expr(items[1]);
                    for (size_t i = 2; i < items.size(); i++) {
                        compile_expr(items[i]);
                        emit_opcode(Opcode::SUB);
                    }
                } else if (op == "*") {
                    if (items.size() < 3)
                        throw std::runtime_error("* requires at least 2 arguments");
                    compile_expr(items[1]);
                    for (size_t i = 2; i < items.size(); i++) {
                        compile_expr(items[i]);
                        emit_opcode(Opcode::MUL);
                    }
                } else if (op == "/") {
                    if (items.size() < 3)
                        throw std::runtime_error("/ requires at least 2 arguments");
                    compile_expr(items[1]);
                    for (size_t i = 2; i < items.size(); i++) {
                        compile_expr(items[i]);
                        emit_opcode(Opcode::DIV);
                    }
                } else if (op == "%") {
                    if (items.size() != 3)
                        throw std::runtime_error("% requires exactly 2 arguments");
                    compile_expr(items[1]);
                    compile_expr(items[2]);
                    emit_opcode(Opcode::MOD);
                }
                // Comparison operators
                else if (op == "=") {
                    if (items.size() != 3)
                        throw std::runtime_error("= requires exactly 2 arguments");
                    compile_expr(items[1]);
                    compile_expr(items[2]);
                    emit_opcode(Opcode::EQ);
                } else if (op == "<") {
                    if (items.size() != 3)
                        throw std::runtime_error("< requires exactly 2 arguments");
                    compile_expr(items[1]);
                    compile_expr(items[2]);
                    emit_opcode(Opcode::LT);
                } else if (op == ">") {
                    if (items.size() != 3)
                        throw std::runtime_error("> requires exactly 2 arguments");
                    compile_expr(items[1]);
                    compile_expr(items[2]);
                    emit_opcode(Opcode::GT);
                }
                // Control flow
                else if (op == "if") {
                    if (items.size() != 4)
                        throw std::runtime_error("if requires 3 arguments: condition, then, else");

                    compile_expr(items[1]); // condition

                    emit_opcode(Opcode::JZ);
                    size_t else_jump = current_address();
                    emit(0); // placeholder for else branch address

                    compile_expr(items[2]); // then branch

                    emit_opcode(Opcode::JMP);
                    size_t end_jump = current_address();
                    emit(0); // placeholder for end address

                    bytecode[else_jump] = current_address(); // patch else jump
                    compile_expr(items[3]);                  // else branch

                    bytecode[end_jump] = current_address(); // patch end jump
                }
                // Sequential evaluation
                else if (op == "do") {
                    if (items.size() < 2)
                        throw std::runtime_error("do requires at least 1 expression");
                    for (size_t i = 1; i < items.size(); i++) {
                        compile_expr(items[i]);
                        if (i < items.size() - 1) {
                            emit_opcode(Opcode::POP); // discard intermediate values
                        }
                    }
                }
                // Print (for debugging)
                else if (op == "print" || op == "print-int") {
                    if (items.size() != 2)
                        throw std::runtime_error("print requires exactly 1 argument");
                    compile_expr(items[1]);
                    emit_opcode(Opcode::PRINT);
                } else if (op == "print-string") {
                    if (items.size() != 2)
                        throw std::runtime_error("print-string requires exactly 1 argument");
                    compile_expr(items[1]);
                    emit_opcode(Opcode::PRINT_STR);
                }
                // Bitwise operations
                else if (op == "bit-and") {
                    if (items.size() != 3)
                        throw std::runtime_error("bit-and requires exactly 2 arguments");
                    compile_expr(items[1]);
                    compile_expr(items[2]);
                    emit_opcode(Opcode::AND);
                } else if (op == "bit-or") {
                    if (items.size() != 3)
                        throw std::runtime_error("bit-or requires exactly 2 arguments");
                    compile_expr(items[1]);
                    compile_expr(items[2]);
                    emit_opcode(Opcode::OR);
                } else if (op == "bit-xor") {
                    if (items.size() != 3)
                        throw std::runtime_error("bit-xor requires exactly 2 arguments");
                    compile_expr(items[1]);
                    compile_expr(items[2]);
                    emit_opcode(Opcode::XOR);
                } else if (op == "bit-shl") {
                    if (items.size() != 3)
                        throw std::runtime_error("bit-shl requires exactly 2 arguments");
                    compile_expr(items[1]);
                    compile_expr(items[2]);
                    emit_opcode(Opcode::SHL);
                } else if (op == "bit-shr") {
                    if (items.size() != 3)
                        throw std::runtime_error("bit-shr requires exactly 2 arguments");
                    compile_expr(items[1]);
                    compile_expr(items[2]);
                    emit_opcode(Opcode::SHR);
                } else if (op == "bit-ashr") {
                    if (items.size() != 3)
                        throw std::runtime_error("bit-ashr requires exactly 2 arguments");
                    compile_expr(items[1]);
                    compile_expr(items[2]);
                    emit_opcode(Opcode::ASHR);
                } else {
                    throw std::runtime_error("Unknown operator: " + op);
                }
                break;
            }
        }
    }

    // (define (func-name param1 param2 ...) body) - Define function
    void compile_define_function(const std::vector<ASTNodePtr>& items) {

        is_in_function = true;

        if (items.size() < 3) {
            throw std::runtime_error("define-func requires at least 2 arguments");
        }

        // Check if it's a function definition: (define (name params...) body)
        if (items[1]->type != NodeType::LIST) {
            throw std::runtime_error("define-func: wrong format");
        }

        const auto& func_def = items[1]->as_list();
        if (func_def.empty() || func_def[0]->type != NodeType::SYMBOL) {
            throw std::runtime_error("define: function name must be a symbol");
        }

        std::string func_name = func_def[0]->as_symbol();

        std::vector<std::string> params;
        for (size_t i = 1; i < func_def.size(); i++) {
            if (func_def[i]->type != NodeType::SYMBOL) {
                throw std::runtime_error("define-func: function parameters must be symbols");
            }
            params.push_back(func_def[i]->as_symbol());
        }

        if (items.size() != 3) {
            throw std::runtime_error("define-func: function requires exactly one body expression");
        }

        // Store function
        Function func;
        func.params = params;
        func.body = items[2];
        func.code_address = 0; // Will be set during compilation
        functions[func_name] = func;

        emit_opcode(Opcode::PUSH);
        emit(0);

        is_in_function = false;
    }

    // (define var value) - Define variable in current scope
    void compile_define_variable(const std::vector<ASTNodePtr>& items) {
        if (items.size() < 3) {
            throw std::runtime_error("define-var requires at least 2 arguments");
        }

        // Variable definition
        if (items[1]->type != NodeType::SYMBOL) {
            throw std::runtime_error("define-var: first argument must be a symbol");
        }

        std::string var_name = items[1]->as_symbol();

        // Define global variable in current scope
        uint64_t addr = 0;
        if (!is_in_function) {
            addr = define_variable(var_name);
        } else {
            addr = define_temporary_variable(var_name);
        }

        // Compile the value expression
        compile_expr(items[2]);

        // Duplicate the value (one for store, one to return)
        emit_opcode(Opcode::DUP);

        // Store it in the variable's memory location
        emit_opcode(Opcode::PUSH);
        emit(addr);
        if (!is_in_function) {
            emit_opcode(Opcode::STORE);
        } else {
            function_temporary_var_index++;
            emit_opcode(Opcode::BP_STORE);
        }
    }

    // (set var value) - Set existing variable
    void compile_set(const std::vector<ASTNodePtr>& items) {
        if (items.size() != 3) {
            throw std::runtime_error("set requires 2 arguments: name and value");
        }
        if (items[1]->type != NodeType::SYMBOL) {
            throw std::runtime_error("set: first argument must be a symbol");
        }

        std::string var_name = items[1]->as_symbol();

        // Look up variable
        std::optional<Variable> addr = lookup_variable(var_name);
        if (!addr) {
            throw std::runtime_error("set: undefined variable: " + var_name);
        }

        // Compile the value expression
        compile_expr(items[2]);

        // Duplicate the value (one for store, one to return)
        emit_opcode(Opcode::DUP);

        // Store it in the variable's memory location
        emit_opcode(Opcode::PUSH);
        emit(addr->addr);
        if (addr->is_global) {
            emit_opcode(Opcode::STORE);
        } else {
            emit_opcode(Opcode::BP_STORE);
        }
        // The duplicated value remains on the stack as the result
    }

    // (let ((var1 val1) (var2 val2) ...) body...)
    void compile_let(const std::vector<ASTNodePtr>& items) {
        if (items.size() < 3) {
            throw std::runtime_error("let requires at least 2 arguments: bindings and body");
        }
        if (items[1]->type != NodeType::LIST) {
            throw std::runtime_error("let: first argument must be a list of bindings");
        }

        const auto& bindings = items[1]->as_list();

        // Push new scope
        push_scope();

        // Process bindings
        for (const auto& binding : bindings) {
            if (binding->type != NodeType::LIST) {
                throw std::runtime_error("let: each binding must be a list");
            }

            const auto& binding_list = binding->as_list();
            if (binding_list.size() != 2) {
                throw std::runtime_error("let: each binding must have exactly 2 elements");
            }
            if (binding_list[0]->type != NodeType::SYMBOL) {
                throw std::runtime_error("let: binding name must be a symbol");
            }

            std::string var_name = binding_list[0]->as_symbol();

            // Compile value expression
            compile_expr(binding_list[1]);

            // Define variable in new scope
            uint64_t addr;
            if (!is_in_function) {
                addr = define_variable(var_name);
            } else {
                addr = define_temporary_variable(var_name);
            }

            // Store the value
            emit_opcode(Opcode::PUSH);
            emit(addr);
            if (!is_in_function) {
                emit_opcode(Opcode::STORE);
            } else {
                function_temporary_var_index++;
                emit_opcode(Opcode::BP_STORE);
            }
        }

        // Compile body expressions
        for (size_t i = 2; i < items.size(); i++) {
            compile_expr(items[i]);
            if (i < items.size() - 1) {
                emit_opcode(Opcode::POP); // discard intermediate values
            }
        }

        // Pop scope
        pop_scope();
    }

    // (while condition body...)
    void compile_while(const std::vector<ASTNodePtr>& items) {
        if (items.size() < 3) {
            throw std::runtime_error("while requires at least 2 arguments: condition and body");
        }

        // Loop structure:
        // loop_start:
        //   evaluate condition
        //   if zero, jump to loop_end
        //   evaluate body (discard result)
        //   jump to loop_start
        // loop_end:
        //   push 0 (while returns 0)

        size_t loop_start = current_address();

        // Evaluate condition
        compile_expr(items[1]);

        // Jump to end if condition is false
        emit_opcode(Opcode::JZ);
        size_t loop_end_ref = current_address();
        emit(0); // Placeholder

        // Compile body
        for (size_t i = 2; i < items.size(); i++) {
            compile_expr(items[i]);
            emit_opcode(Opcode::POP); // Discard body results
        }

        // Jump back to loop start
        emit_opcode(Opcode::JMP);
        emit(loop_start);

        // Patch loop end address
        bytecode[loop_end_ref] = current_address();

        // Push 0 as return value (while doesn't return anything useful)
        emit_opcode(Opcode::PUSH);
        emit(0);
    }

    // (for (var start end) body...)
    // Iterates var from start to end-1
    void compile_for(const std::vector<ASTNodePtr>& items) {
        if (items.size() < 3) {
            throw std::runtime_error("for requires at least 2 arguments: (var start end) and body");
        }
        if (items[1]->type != NodeType::LIST) {
            throw std::runtime_error("for: first argument must be (var start end)");
        }

        const auto& loop_spec = items[1]->as_list();
        if (loop_spec.size() != 3) {
            throw std::runtime_error("for: loop spec must be (var start end)");
        }
        if (loop_spec[0]->type != NodeType::SYMBOL) {
            throw std::runtime_error("for: loop variable must be a symbol");
        }

        std::string var_name = loop_spec[0]->as_symbol();

        // Push new scope for loop variable
        push_scope();

        // Initialize loop variable
        compile_expr(loop_spec[1]); // start value
        uint64_t var_addr = define_variable(var_name);
        emit_opcode(Opcode::PUSH);
        emit(var_addr);
        emit_opcode(Opcode::STORE);

        // Evaluate and store end value in a temp variable
        compile_expr(loop_spec[2]); // end value
        uint64_t end_addr = define_variable("__for_end__");
        emit_opcode(Opcode::PUSH);
        emit(end_addr);
        emit_opcode(Opcode::STORE);

        // Loop structure:
        // loop_start:
        //   load var
        //   load end
        //   if var >= end, jump to loop_end
        //   evaluate body (discard result)
        //   increment var
        //   jump to loop_start
        // loop_end:
        //   push 0 (for returns 0)

        size_t loop_start = current_address();

        // Check condition: var < end
        emit_opcode(Opcode::PUSH);
        emit(var_addr);
        emit_opcode(Opcode::LOAD);

        emit_opcode(Opcode::PUSH);
        emit(end_addr);
        emit_opcode(Opcode::LOAD);

        emit_opcode(Opcode::LT); // var < end

        // Jump to end if condition is false
        emit_opcode(Opcode::JZ);
        size_t loop_end_ref = current_address();
        emit(0); // Placeholder

        // Compile body
        for (size_t i = 2; i < items.size(); i++) {
            compile_expr(items[i]);
            emit_opcode(Opcode::POP); // Discard body results
        }

        // Increment loop variable: var = var + 1
        emit_opcode(Opcode::PUSH);
        emit(var_addr);
        emit_opcode(Opcode::LOAD);

        emit_opcode(Opcode::PUSH);
        emit(1);

        emit_opcode(Opcode::ADD);

        emit_opcode(Opcode::PUSH);
        emit(var_addr);
        emit_opcode(Opcode::STORE);

        // Jump back to loop start
        emit_opcode(Opcode::JMP);
        emit(loop_start);

        // Patch loop end address
        bytecode[loop_end_ref] = current_address();

        // Pop scope
        pop_scope();

        // Push 0 as return value
        emit_opcode(Opcode::PUSH);
        emit(0);
    }

    // (peek addr) - Read memory at address
    void compile_peek(const std::vector<ASTNodePtr>& items) {
        if (items.size() != 2) {
            throw std::runtime_error("peek requires 1 argument: address");
        }

        // Compile address expression
        compile_expr(items[1]);

        // LOAD from that address
        emit_opcode(Opcode::LOAD);
    }

    // (poke addr value) - Write value to memory at address
    void compile_poke(const std::vector<ASTNodePtr>& items) {
        if (items.size() != 3) {
            throw std::runtime_error("poke requires 2 arguments: address and value");
        }

        // STORE expects stack: [value, address] (address on top)
        // So push value first, then address

        // Compile value expression
        compile_expr(items[2]);

        // Compile address expression
        compile_expr(items[1]);

        // STORE value to address
        emit_opcode(Opcode::STORE);

        // Return the value (for consistency)
        compile_expr(items[2]);
    }

    // Compile string literal into memory at runtime
    void compile_string_literal(const std::string& str) {
        // Generate code to:
        // 1. Allocate memory for string
        // 2. Write length
        // 3. Write characters (packed 8 per word)
        // 4. Return address

        const size_t len = str.length();

        // We need to call malloc at runtime
        // For now, generate a compile-time allocated string in a global area
        // This is a simplified approach - ideally we'd generate runtime allocation code

        // Allocate a variable to hold the string address
        std::string var_name = "__string_" + std::to_string(current_address()) + "__";
        uint64_t str_addr = define_variable(var_name);

        // Generate initialization code (needs to run once at startup)
        // For simplicity, we'll store string data in consecutive variable slots

        // Store length
        emit_opcode(Opcode::PUSH);
        emit(len);
        emit_opcode(Opcode::PUSH);
        emit(str_addr);
        emit_opcode(Opcode::STORE);

        // Pack and store characters
        for (size_t word_idx = 0; word_idx < (len + 7) / 8; word_idx++) {
            uint64_t word = 0;
            for (size_t byte_idx = 0; byte_idx < 8 && (word_idx * 8 + byte_idx) < len; byte_idx++) {
                uint8_t ch = str[word_idx * 8 + byte_idx];
                word |= (static_cast<uint64_t>(ch) << (byte_idx * 8));
            }

            uint64_t data_addr = define_variable(var_name + "_" + std::to_string(word_idx));
            emit_opcode(Opcode::PUSH);
            emit(word);
            emit_opcode(Opcode::PUSH);
            emit(data_addr);
            emit_opcode(Opcode::STORE);
        }

        // Push the string address (variable holding the string start)
        emit_opcode(Opcode::PUSH);
        emit(str_addr);
    }

    // Compile function call
    void compile_function_call(const std::string& func_name, const std::vector<ASTNodePtr>& items) {
        const Function& func = functions[func_name];

        // Check argument count
        size_t num_args = items.size() - 1;
        if (num_args != func.params.size()) {
            throw std::runtime_error("Function " + func_name + " expects " +
                                     std::to_string(func.params.size()) + " arguments, got " +
                                     std::to_string(num_args));
        }

        // Evaluate arguments and push onto stack
        for (size_t i = 0; i < num_args; i++) {
            compile_expr(items[i + 1]);
        }

        // Call the function
        // Arguments are now on the stack
        emit_opcode(Opcode::CALL);

        // Placeholder for function address
        size_t call_addr_pos = current_address();
        emit(0);

        // Number of arguments
        emit(num_args);

        // Store label reference for later patching
        label_refs.push_back({call_addr_pos, func_name});

        // Result is now on stack
    }

  public:
    LispCompiler()
        : is_in_function(false), next_var_address(VAR_START), function_local_var_index(0),
          function_temporary_var_index(0) {
        // Start with global scope
        push_scope();
    }

    std::vector<uint64_t> compile(const ASTNodePtr& ast) {
        bytecode.clear();
        labels.clear();
        label_refs.clear();

        compile_expr(ast);
        emit_opcode(Opcode::HALT);

        // Compile functions
        compile_all_functions();

        // Patch function calls
        patch_function_calls();

        return bytecode;
    }

    std::vector<uint64_t> compile_program(const std::vector<ASTNodePtr>& exprs) {
        bytecode.clear();
        labels.clear();
        label_refs.clear();

        for (size_t i = 0; i < exprs.size(); i++) {
            compile_expr(exprs[i]);
            if (i < exprs.size() - 1) {
                emit_opcode(Opcode::POP); // discard intermediate results
            }
        }

        emit_opcode(Opcode::HALT);

        // Compile functions
        compile_all_functions();

        // Patch function calls
        patch_function_calls();

        return bytecode;
    }

    // Reset compiler state for new program
    void reset() {
        scopes.clear();
        push_scope(); // Re-establish global scope
        next_var_address = VAR_START;
        function_local_var_index = 0;
        function_temporary_var_index = 0;
        is_in_function = false;
        functions.clear();
    }

  private:
    // Compile all function definitions
    void compile_all_functions() {
        for (auto& [name, func] : functions) {

            // Record function start address
            func.code_address = current_address();
            labels[name] = func.code_address;

            push_scope();

            // Now pop arguments and store them
            std::vector<uint64_t> param_addrs;
            for (const auto& param : func.params) {
                define_argument_variable(param);
            }

            emit_opcode(Opcode::ENTER);
            uint64_t patchTemporaries = current_address();
            // placeholder for temporaries
            emit(0);

            // Compile function body
            compile_expr(func.body);

            emit_opcode(Opcode::LEAVE);
            emit(function_temporary_var_index);
            bytecode[patchTemporaries] = function_temporary_var_index;

            // Return
            emit_opcode(Opcode::RET);
            emit(func.params.size());

            pop_scope();

            function_local_var_index = 0;
            function_temporary_var_index = 0;
        }
    }

    // Patch all function call addresses
    void patch_function_calls() {
        for (const auto& [pos, func_name] : label_refs) {
            if (labels.find(func_name) == labels.end()) {
                throw std::runtime_error("Undefined function: " + func_name);
            }
            bytecode[pos] = labels[func_name];
        }
    }
};
