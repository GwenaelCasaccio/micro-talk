#pragma once
#include "symbol_table.hpp"
#include <cstdint>
#include <functional>
#include <string>

// Forward declaration
class StackVM;

// ============================================================================
// EvalContext - Shared context for runtime eval/compile
// ============================================================================
// This structure holds the compilation state shared between the VM and the
// EvalService. It allows dynamic code generation by providing access to
// the symbol table and address allocators.
//
// The callbacks break the circular dependency between VM and compiler:
// - VM calls the callback when EVAL/COMPILE opcode is executed
// - Callback is implemented in main.cpp where both headers are available

struct EvalContext {
    SymbolTable* symbols;          // Shared symbol table
    uint64_t* next_var_address;    // Where to allocate next variable
    uint64_t* next_string_address; // Where to allocate next string
    uint64_t* next_code_address;   // Where to load compiled code

    // Callback: compile string and load into VM, return code address
    // The code ends with HALT (for eval)
    std::function<uint64_t(StackVM&, const std::string&)> compile_for_eval;

    // Callback: compile string and load into VM, return code address
    // The code ends with RET (for funcall)
    std::function<uint64_t(StackVM&, const std::string&)> compile_for_funcall;
};
