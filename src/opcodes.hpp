#pragma once
#include <cstdint>

// ============================================================================
// Opcode Definitions
// ============================================================================
// Virtual machine instruction set for the stack-based VM.
// Each opcode is encoded as a single byte (uint8_t).

enum class Opcode : uint8_t {
    HALT = 0,
    PUSH,       // Push immediate 64-bit value
    POP,        // Pop and discard
    DUP,        // Duplicate top
    SWAP,       // Swap top two elements
    ADD,        // Pop two, push sum
    SUB,        // Pop two, push difference
    MUL,        // Pop two, push product
    DIV,        // Pop two, push quotient
    MOD,        // Pop two, push remainder
    EQ,         // Pop two, push 1 if equal, 0 otherwise
    LT,         // Pop two, push 1 if less than, 0 otherwise
    GT,         // Pop two, push 1 if greater than, 0 otherwise
    LTE,        // Pop two, push 1 if less than or equal, 0 otherwise
    GTE,        // Pop two, push 1 if greater than or equal, 0 otherwise
    JMP,        // Unconditional jump to address
    JZ,         // Jump if top of stack is zero
    ENTER,      // Save previous BP to the stack set it to SP
    LEAVE,      // Restore BP from the stack
    CALL,       // Call function at address
    RET,        // Return from function
    IRET,       // Return from interruption
    LOAD,       // Load 64-bit word from memory address on stack
    STORE,      // Store 64-bit word to memory address
    LOAD_BYTE,  // Load byte from memory (address on stack)
    STORE_BYTE, // Store byte to memory (value, address on stack)
    LOAD32,     // Load 32-bit word from memory
    STORE32,    // Store 32-bit word to memory
    BP_LOAD,    // Load from base pointer offset
    BP_STORE,   // Store to base pointer offset
    PRINT,      // Debug: print top of stack as integer
    PRINT_STR,  // Debug: print string at address on stack
    AND,        // Bitwise AND
    OR,         // Bitwise OR
    XOR,        // Bitwise XOR
    SHL,        // Shift left
    SHR,        // Shift right (logical)
    ASHR,       // Arithmetic shift right
    CLI,        // Clear interrupt flag
    STI,        // Set interrupt flag
    SIGNAL_REG, // Register signal handler
    ABORT,      // Abort with error message (address on stack)
    FUNCALL,    // Call function at address on stack (address, arg_count on stack)
    EVAL,       // Pop string addr, compile+execute, push result
    COMPILE,    // Pop string addr, compile, push code address
    C_CALL,     // Call C function: [func_id, arg_count, args...] -> [result]
};

// ============================================================================
// Opcode Utilities
// ============================================================================

// Convert opcode to human-readable string name for debugging/disassembly
inline const char* opcode_name(Opcode op) {
    switch (op) {
        case Opcode::HALT:
            return "HALT";
        case Opcode::PUSH:
            return "PUSH";
        case Opcode::POP:
            return "POP";
        case Opcode::DUP:
            return "DUP";
        case Opcode::SWAP:
            return "SWAP";
        case Opcode::ADD:
            return "ADD";
        case Opcode::SUB:
            return "SUB";
        case Opcode::MUL:
            return "MUL";
        case Opcode::DIV:
            return "DIV";
        case Opcode::MOD:
            return "MOD";
        case Opcode::EQ:
            return "EQ";
        case Opcode::LT:
            return "LT";
        case Opcode::GT:
            return "GT";
        case Opcode::LTE:
            return "LTE";
        case Opcode::GTE:
            return "GTE";
        case Opcode::JMP:
            return "JMP";
        case Opcode::JZ:
            return "JZ";
        case Opcode::ENTER:
            return "ENTER";
        case Opcode::LEAVE:
            return "LEAVE";
        case Opcode::CALL:
            return "CALL";
        case Opcode::RET:
            return "RET";
        case Opcode::IRET:
            return "IRET";
        case Opcode::LOAD:
            return "LOAD";
        case Opcode::STORE:
            return "STORE";
        case Opcode::LOAD_BYTE:
            return "LOAD_BYTE";
        case Opcode::STORE_BYTE:
            return "STORE_BYTE";
        case Opcode::LOAD32:
            return "LOAD32";
        case Opcode::STORE32:
            return "STORE32";
        case Opcode::BP_LOAD:
            return "BP_LOAD";
        case Opcode::BP_STORE:
            return "BP_STORE";
        case Opcode::PRINT:
            return "PRINT";
        case Opcode::PRINT_STR:
            return "PRINT_STR";
        case Opcode::AND:
            return "AND";
        case Opcode::OR:
            return "OR";
        case Opcode::XOR:
            return "XOR";
        case Opcode::SHL:
            return "SHL";
        case Opcode::SHR:
            return "SHR";
        case Opcode::ASHR:
            return "ASHR";
        case Opcode::CLI:
            return "CLI";
        case Opcode::STI:
            return "STI";
        case Opcode::SIGNAL_REG:
            return "SIGNAL_REG";
        case Opcode::ABORT:
            return "ABORT";
        case Opcode::FUNCALL:
            return "FUNCALL";
        case Opcode::EVAL:
            return "EVAL";
        case Opcode::COMPILE:
            return "COMPILE";
        case Opcode::C_CALL:
            return "C_CALL";
        default:
            return "UNKNOWN";
    }
}
