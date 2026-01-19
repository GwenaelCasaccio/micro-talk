#include "eval_context.hpp"
#include "lisp_compiler.hpp"
#include "lisp_parser.hpp"
#include "stack_vm.hpp"
#include "symbol_table.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

// ============================================================================
// Persistent REPL State
// ============================================================================

struct REPLState {
    StackVM vm;
    SymbolTable symbols;
    uint64_t next_var_address;
    uint64_t next_string_address;
    uint64_t next_code_address; // Track where to load next bytecode
    EvalContext eval_ctx;       // Context for runtime eval/compile

    static constexpr uint64_t GLOBALS_BASE = 134217728;
    static constexpr uint64_t STRING_TABLE_START = GLOBALS_BASE;
    static constexpr uint64_t VAR_START = GLOBALS_BASE + 100000000;

    REPLState()
        : next_var_address(VAR_START), next_string_address(STRING_TABLE_START),
          next_code_address(0) {
        // Initialize eval context
        eval_ctx.symbols = &symbols;
        eval_ctx.next_var_address = &next_var_address;
        eval_ctx.next_string_address = &next_string_address;
        eval_ctx.next_code_address = &next_code_address;

        // Set up callback for EVAL opcode (compile with RET for execution)
        eval_ctx.compile_for_eval = [this](StackVM& /*vm*/, const std::string& code) -> uint64_t {
            return compile_code_for_call(code);
        };

        // Set up callback for COMPILE opcode (compile with RET for later funcall)
        eval_ctx.compile_for_funcall = [this](StackVM& /*vm*/,
                                              const std::string& code) -> uint64_t {
            return compile_code_for_call(code);
        };

        // Set the eval context on the VM
        vm.set_eval_context(&eval_ctx);
    }

    void reset() {
        vm.reset();
        symbols.clear();
        next_var_address = VAR_START;
        next_string_address = STRING_TABLE_START;
        next_code_address = 0;
        // Re-establish eval context on VM after reset
        vm.set_eval_context(&eval_ctx);
    }

    // Helper: compile code that can be called (ends with RET 0)
    uint64_t compile_code_for_call(const std::string& code) {
        LispParser parser(code);
        auto ast = parser.parse();

        LispCompiler compiler;
        compiler.import_symbols(symbols);
        compiler.set_next_var_address(next_var_address);
        compiler.set_next_string_address(next_string_address);
        compiler.set_code_base_address(next_code_address);

        // Compile as function (ends with RET 0)
        auto program = compiler.compile_as_function(ast);

        uint64_t code_addr = next_code_address;
        vm.load_program_at(program, code_addr);
        compiler.write_strings_to_memory(vm);

        // Update state with new symbols
        symbols.merge(compiler.get_symbol_table());
        next_var_address = compiler.get_next_var_address();
        next_string_address = compiler.get_next_string_address();
        next_code_address += program.bytecode.size();

        return code_addr;
    }

    bool compile_and_execute(const std::string& source, bool verbose = false);
};

bool REPLState::compile_and_execute(const std::string& source, bool verbose) {
    try {
        // Parse
        LispParser parser(source);
        auto ast = parser.parse();

        // Create a new compiler for this expression
        LispCompiler compiler;

        // Import existing symbols from previous compilations
        compiler.import_symbols(symbols);
        compiler.set_next_var_address(next_var_address);
        compiler.set_next_string_address(next_string_address);
        compiler.set_code_base_address(next_code_address);

        // Compile
        auto program = compiler.compile(ast);

        if (verbose) {
            std::cout << "Bytecode (" << program.bytecode.size() << " words) @ "
                      << next_code_address << ":" << '\n';
            for (size_t i = 0; i < program.bytecode.size(); i++) {
                std::cout << "  " << (next_code_address + i) << ": " << program.bytecode[i] << '\n';
            }
            std::cout << std::flush;
        }

        // Reset VM IP to start of new code (preserves memory contents)
        vm.reset();

        // Load at the code base address (appending to existing code)
        vm.load_program_at(program, next_code_address);
        compiler.write_strings_to_memory(vm);

        // Update next_code_address BEFORE execute so that any nested
        // compile_for_eval calls don't overwrite the current program
        uint64_t start_addr = next_code_address;
        next_code_address += program.bytecode.size();

        // Set IP to start of this code block
        vm.set_ip(start_addr);

        if (verbose) {
            std::cout << "Starting execution at IP=" << start_addr << '\n';
            std::cout << std::flush;
        }

        // Execute with instruction limit for safety (prevent infinite loops)
        vm.execute(1000000);

        // Update persistent state with new symbols
        symbols.merge(compiler.get_symbol_table());
        next_var_address = compiler.get_next_var_address();
        next_string_address = compiler.get_next_string_address();

        // Print result
        std::cout << "=> " << static_cast<int64_t>(vm.get_top()) << '\n';
        return true;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return false;
    }
}

// ============================================================================
// REPL Command Handlers
// ============================================================================

void cmd_help() {
    std::cout << "REPL Commands:" << '\n';
    std::cout << "  :help          - Show this help message" << '\n';
    std::cout << "  :symbols       - List all defined symbols" << '\n';
    std::cout << "  :vars          - List variables with current values" << '\n';
    std::cout << "  :funcs         - List functions with signatures" << '\n';
    std::cout << "  :eval <name>   - Print value of variable" << '\n';
    std::cout << "  :set <name> <value>  - Set variable to new value" << '\n';
    std::cout << "  :reset         - Clear all state" << '\n';
    std::cout << "  :verbose       - Toggle verbose mode (show bytecode)" << '\n';
    std::cout << "  quit / exit    - Exit the REPL" << '\n';
}

void cmd_symbols(const REPLState& state) {
    auto all = state.symbols.all_symbols();
    if (all.empty()) {
        std::cout << "No symbols defined." << '\n';
        return;
    }

    std::cout << "Defined symbols (" << all.size() << "):" << '\n';
    for (const auto& entry : all) {
        if (entry.is_variable()) {
            std::cout << "  var  " << entry.name << " @ " << entry.address << '\n';
        } else {
            std::cout << "  func " << entry.name << "(";
            for (size_t i = 0; i < entry.params.size(); i++) {
                if (i > 0)
                    std::cout << ", ";
                std::cout << entry.params[i];
            }
            std::cout << ") @ " << entry.address << '\n';
        }
    }
}

void cmd_vars(REPLState& state) {
    auto vars = state.symbols.all_variables();
    if (vars.empty()) {
        std::cout << "No variables defined." << '\n';
        return;
    }

    std::cout << "Variables (" << vars.size() << "):" << '\n';
    for (const auto& entry : vars) {
        int64_t value = static_cast<int64_t>(state.vm.read_memory(entry.address));
        std::cout << "  " << entry.name << " @ " << entry.address << " = " << value << '\n';
    }
}

void cmd_funcs(const REPLState& state) {
    auto funcs = state.symbols.all_functions();
    if (funcs.empty()) {
        std::cout << "No functions defined." << '\n';
        return;
    }

    std::cout << "Functions (" << funcs.size() << "):" << '\n';
    for (const auto& entry : funcs) {
        std::cout << "  (" << entry.name;
        for (const auto& p : entry.params) {
            std::cout << " " << p;
        }
        std::cout << ") @ " << entry.address << '\n';
    }
}

void cmd_eval(REPLState& state, const std::string& name) {
    auto entry = state.symbols.lookup(name);
    if (!entry.has_value()) {
        std::cerr << "Error: Unknown symbol: " << name << '\n';
        return;
    }

    if (entry->is_function()) {
        std::cerr << "Error: '" << name << "' is a function, not a variable" << '\n';
        return;
    }

    int64_t value = static_cast<int64_t>(state.vm.read_memory(entry->address));
    std::cout << name << " = " << value << '\n';
}

void cmd_set(REPLState& state, const std::string& name, const std::string& value_str) {
    auto entry = state.symbols.lookup(name);
    if (!entry.has_value()) {
        std::cerr << "Error: Unknown symbol: " << name << '\n';
        return;
    }

    if (entry->is_function()) {
        std::cerr << "Error: '" << name << "' is a function, cannot set its value" << '\n';
        return;
    }

    try {
        int64_t value = std::stoll(value_str);
        state.vm.write_memory(entry->address, static_cast<uint64_t>(value));
        std::cout << name << " = " << value << '\n';
    } catch (const std::exception&) {
        std::cerr << "Error: Invalid number: " << value_str << '\n';
    }
}

// ============================================================================
// Command Parsing and Dispatch
// ============================================================================

bool handle_command(REPLState& state, const std::string& line, bool& verbose) {
    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;

    if (cmd == ":help") {
        cmd_help();
        return true;
    } else if (cmd == ":symbols") {
        cmd_symbols(state);
        return true;
    } else if (cmd == ":vars") {
        cmd_vars(state);
        return true;
    } else if (cmd == ":funcs") {
        cmd_funcs(state);
        return true;
    } else if (cmd == ":eval") {
        std::string name;
        if (iss >> name) {
            cmd_eval(state, name);
        } else {
            std::cerr << "Usage: :eval <name>" << '\n';
        }
        return true;
    } else if (cmd == ":set") {
        std::string name, value;
        if (iss >> name >> value) {
            cmd_set(state, name, value);
        } else {
            std::cerr << "Usage: :set <name> <value>" << '\n';
        }
        return true;
    } else if (cmd == ":reset") {
        state.reset();
        std::cout << "REPL state cleared." << '\n';
        return true;
    } else if (cmd == ":verbose") {
        verbose = !verbose;
        std::cout << "Verbose mode: " << (verbose ? "ON" : "OFF") << '\n';
        return true;
    }

    return false; // Not a command
}

// ============================================================================
// Demo Functions (for initial demonstration)
// ============================================================================

void run_demo(REPLState& state) {
    std::cout << "=== Minimal Stack VM + Lisp Demo ===" << '\n' << '\n';

    std::cout << "Basic arithmetic:" << '\n';
    state.compile_and_execute("(+ 5 3)");
    state.compile_and_execute("(* 7 6)");
    state.compile_and_execute("(- 20 8)");
    state.compile_and_execute("(/ 100 5)");

    std::cout << '\n' << "Nested expressions:" << '\n';
    state.compile_and_execute("(+ (* 3 4) 5)");
    state.compile_and_execute("(* (+ 2 3) (- 10 4))");

    std::cout << '\n' << "Comparison:" << '\n';
    state.compile_and_execute("(< 5 10)");
    state.compile_and_execute("(= 7 7)");
    state.compile_and_execute("(> 3 8)");

    std::cout << '\n' << "Conditional:" << '\n';
    state.compile_and_execute("(if (< 5 10) 100 200)");
    state.compile_and_execute("(if (> 5 10) 100 200)");

    std::cout << '\n' << "Complex expression:" << '\n';
    state.compile_and_execute("(if (= (% 10 3) 1) (* 5 5) (+ 1 1))");

    std::cout << '\n' << "Sequential execution with print:" << '\n';
    state.compile_and_execute("(do (print 42) (print 99) (+ 1 2 3))");

    // Reset state for interactive REPL
    state.reset();
    std::cout << '\n';
}

// ============================================================================
// Main Entry Point
// ============================================================================

// Load and execute a file, returns true on success
bool load_file(REPLState& state, const std::string& filename, bool verbose = false) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file: " << filename << '\n';
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    if (verbose) {
        std::cout << "Loading: " << filename << " (" << content.size() << " bytes)" << '\n';
    }

    return state.compile_and_execute(content, verbose);
}

int main(int argc, char* argv[]) {
    REPLState state;
    bool verbose = false;

    // Check for flags and files
    std::vector<std::string> files;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-v" || arg == "--verbose") {
            verbose = true;
        } else if (arg == "-h" || arg == "--help") {
            std::cout << "Usage: " << argv[0] << " [options] [file1.lisp file2.lisp ...]" << '\n';
            std::cout << "Options:" << '\n';
            std::cout << "  -v, --verbose  Show bytecode during execution" << '\n';
            std::cout << "  -h, --help     Show this help message" << '\n';
            std::cout << '\n';
            std::cout << "If no files are provided, runs interactive REPL." << '\n';
            return 0;
        } else {
            files.push_back(arg);
        }
    }

    // If files provided, execute them in order
    if (!files.empty()) {
        for (const auto& filename : files) {
            if (!load_file(state, filename, verbose)) {
                return 1;
            }
        }
        return 0;
    }

    // No files - run demo and interactive REPL
    run_demo(state);

    std::cout << "=== Interactive REPL (type ':help' for commands, 'quit' to exit) ===" << '\n';
    std::string line;

    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line))
            break;

        // Trim whitespace
        size_t start = line.find_first_not_of(" \t");
        if (start == std::string::npos) {
            continue; // Empty line
        }
        line = line.substr(start);

        // Check for exit
        if (line == "quit" || line == "exit") {
            break;
        }

        // Check for REPL commands (starting with :)
        if (!line.empty() && line[0] == ':') {
            handle_command(state, line, verbose);
            continue;
        }

        // Evaluate as Lisp expression
        state.compile_and_execute(line, verbose);
    }

    return 0;
}
