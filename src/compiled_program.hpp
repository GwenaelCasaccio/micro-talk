#pragma once
#include <cstdint>
#include <string>
#include <vector>

// ============================================================================
// Compiled Program Structure
// ============================================================================
// Represents a compiled Lisp program with bytecode and associated data.
// Used to package compilation results before loading into the VM.

// Forward declaration
class StackVM;

// Compiled program with bytecode and data
struct CompiledProgram {
    // Executable bytecode
    std::vector<uint64_t> bytecode;

    // String literal metadata for runtime initialization
    struct StringLiteral {
        std::string content; // The actual string content
        uint64_t address;    // Heap address where string should be stored
    };
    std::vector<StringLiteral> strings;

    // Write string data to VM memory
    // Note: Implementation is defined after StackVM class (see stack_vm.hpp)
    void write_strings(StackVM& vm) const;
};
