#pragma once
#include "stack_vm.hpp"
#include "lisp_parser.hpp"
#include "lisp_compiler.hpp"
#include <map>
#include <string>
#include <vector>
#include <memory>

// Microcode system for extending the VM with Lisp-defined instructions
class MicrocodeSystem {
public:
    struct Microcode {
        std::string name;
        uint8_t opcode;
        int param_count;  // Number of parameters (from stack)
        std::vector<uint64_t> bytecode;  // Compiled Lisp code
        
        Microcode() : opcode(0), param_count(0) {}
    };
    
private:
    std::map<uint8_t, Microcode> instructions;
    std::map<std::string, uint8_t> name_to_opcode;
    uint8_t next_opcode;
    
    static constexpr uint8_t MICROCODE_START = 100;
    static constexpr uint8_t MICROCODE_END = 255;
    
public:
    MicrocodeSystem() : next_opcode(MICROCODE_START) {}
    
    // Define a new microcode instruction
    uint8_t define(const std::string& name, int param_count, 
                   const std::vector<uint64_t>& bytecode) {
        if (next_opcode >= MICROCODE_END) {
            throw std::runtime_error("Microcode table full (max 155 instructions)");
        }
        
        if (name_to_opcode.find(name) != name_to_opcode.end()) {
            throw std::runtime_error("Microcode already defined: " + name);
        }
        
        Microcode mc;
        mc.name = name;
        mc.opcode = next_opcode;
        mc.param_count = param_count;
        mc.bytecode = bytecode;
        
        instructions[next_opcode] = mc;
        name_to_opcode[name] = next_opcode;
        
        return next_opcode++;
    }
    
    // Check if opcode is a microcode instruction
    bool is_microcode(uint8_t opcode) const {
        return instructions.find(opcode) != instructions.end();
    }
    
    // Get microcode by opcode
    const Microcode* get(uint8_t opcode) const {
        auto it = instructions.find(opcode);
        return (it != instructions.end()) ? &it->second : nullptr;
    }
    
    // Get opcode by name
    bool get_opcode(const std::string& name, uint8_t& opcode) const {
        auto it = name_to_opcode.find(name);
        if (it != name_to_opcode.end()) {
            opcode = it->second;
            return true;
        }
        return false;
    }
    
    // List all microcode instructions
    std::vector<std::string> list() const {
        std::vector<std::string> names;
        for (const auto& [opcode, mc] : instructions) {
            names.push_back(mc.name + " (opcode " + std::to_string(opcode) + 
                          ", " + std::to_string(mc.param_count) + " params)");
        }
        return names;
    }
    
    void print() const {
        printf("=== Microcode Instructions ===\n");
        for (const auto& [opcode, mc] : instructions) {
            printf("%3d: %-20s (%d params, %zu words)\n", 
                   opcode, mc.name.c_str(), mc.param_count, mc.bytecode.size());
        }
    }
};

// Microcode-aware compiler
class MicrocodeCompiler {
private:
    MicrocodeSystem& microcode_sys;
    LispCompiler base_compiler;
    
public:
    MicrocodeCompiler(MicrocodeSystem& ms) : microcode_sys(ms) {}
    
    // Parse and compile a defmicro definition
    // (defmicro name (param1 param2 ...) body)
    uint8_t compile_defmicro(const std::string& source) {
        LispParser parser(source);
        auto ast = parser.parse();
        
        if (ast->type != NodeType::LIST) {
            throw std::runtime_error("defmicro: expected list");
        }
        
        const auto& items = ast->as_list();
        if (items.size() != 4) {
            throw std::runtime_error("defmicro: expected (defmicro name (params) body)");
        }
        
        if (items[0]->type != NodeType::SYMBOL || items[0]->as_symbol() != "defmicro") {
            throw std::runtime_error("defmicro: expected defmicro keyword");
        }
        
        if (items[1]->type != NodeType::SYMBOL) {
            throw std::runtime_error("defmicro: name must be a symbol");
        }
        
        if (items[2]->type != NodeType::LIST) {
            throw std::runtime_error("defmicro: params must be a list");
        }
        
        std::string name = items[1]->as_symbol();
        const auto& params = items[2]->as_list();
        ASTNodePtr body = items[3];
        
        // Create a function that implements the microcode
        // (define (name param1 param2 ...) body)
        std::vector<ASTNodePtr> func_def;
        func_def.push_back(ASTNode::make_symbol("define"));
        
        // Function signature
        std::vector<ASTNodePtr> signature;
        signature.push_back(ASTNode::make_symbol(name));
        for (const auto& param : params) {
            if (param->type != NodeType::SYMBOL) {
                throw std::runtime_error("defmicro: all params must be symbols");
            }
            signature.push_back(param);
        }
        func_def.push_back(ASTNode::make_list(signature));
        func_def.push_back(body);
        
        auto func_ast = ASTNode::make_list(func_def);
        
        // Compile the function
        LispCompiler compiler;
        auto bytecode = compiler.compile(func_ast);
        
        // Register as microcode
        uint8_t opcode = microcode_sys.define(name, params.size(), bytecode);
        
        printf("Defined microcode: %s (opcode %d, %zu params)\n", 
               name.c_str(), opcode, params.size());
        
        return opcode;
    }
    
    // Compile regular Lisp code that may use microcode instructions
    std::vector<uint64_t> compile(const std::string& source) {
        // For now, just use the base compiler
        LispParser parser(source);
        auto ast = parser.parse();
        return base_compiler.compile(ast);
    }
};

// Example microcode library for Smalltalk-like operations
namespace SmalltalkMicrocode {
    // Generate standard Smalltalk microcodes
    void define_smalltalk_primitives(MicrocodeCompiler& compiler) {
        // Object creation: new-object
        compiler.compile_defmicro(
            "(defmicro new-object (class-id) "
            "  (do "
            "    (define TAG_OOP 0) "
            "    (define obj-addr 16384) "
            "    (bit-or obj-addr TAG_OOP)))");
        
        // Send message (simplified): send
        compiler.compile_defmicro(
            "(defmicro send-msg (receiver selector) "
            "  (do "
            "    (define TAG_MASK 7) "
            "    (define is-oop-check (= (bit-and receiver TAG_MASK) 0)) "
            "    (if is-oop-check receiver 0)))");
        
        // Get class of object
        compiler.compile_defmicro(
            "(defmicro get-class (obj) "
            "  (do "
            "    (define addr (bit-and obj (bit-xor -1 7))) "
            "    (define class-id addr) "
            "    class-id)))");
        
        // Array access
        compiler.compile_defmicro(
            "(defmicro array-at (array index) "
            "  (do "
            "    (define base (bit-and array (bit-xor -1 7))) "
            "    (define offset (+ base index)) "
            "    offset)))");
    }
}
