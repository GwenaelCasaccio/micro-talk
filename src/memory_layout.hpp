#pragma once

#include <cstddef>

// Memory layout constants for the stack-based virtual machine
//
// The VM uses a flat 4GB mmap-allocated memory space divided into four regions:
// [0 ... CODE_SIZE)                    : Code segment (1GB)
// [GLOBALS_START ... HEAP_START)       : Global variables (1GB)
// [HEAP_START ... STACK_START)         : Heap (1GB, grows upward)
// [STACK_START ... MEMORY_SIZE)        : Stack (1GB, grows downward)
//
// Uses virtual memory - physical memory only allocated on access

namespace MemoryLayout {
// Base unit: 1GB in 64-bit words
constexpr size_t GB_IN_WORDS = 134217728; // 1GB / 8 bytes = 128M words

// Memory region boundaries (in 64-bit words)
constexpr size_t CODE_SIZE = GB_IN_WORDS;       // 1GB for code
constexpr size_t GLOBALS_START = CODE_SIZE;     // Starts after code
constexpr size_t HEAP_START = 2 * GB_IN_WORDS;  // Starts after globals
constexpr size_t STACK_START = 3 * GB_IN_WORDS; // Starts after heap
constexpr size_t MEMORY_SIZE = 4 * GB_IN_WORDS; // 4GB total

// Stack grows downward from this base
constexpr size_t STACK_BASE = STACK_START;

// Helper functions for region checks
inline constexpr bool is_in_code_segment(size_t addr) {
    return addr < CODE_SIZE;
}

inline constexpr bool is_in_globals(size_t addr) {
    return addr >= GLOBALS_START && addr < HEAP_START;
}

inline constexpr bool is_in_heap(size_t addr) {
    return addr >= HEAP_START && addr < STACK_START;
}

inline constexpr bool is_in_stack(size_t addr) {
    return addr >= STACK_START && addr < MEMORY_SIZE;
}

inline constexpr bool is_valid_address(size_t addr) {
    return addr < MEMORY_SIZE;
}

// Convert bytes to words
inline constexpr size_t bytes_to_words(size_t bytes) {
    return (bytes + 7) / 8; // Round up
}

// Convert words to bytes
inline constexpr size_t words_to_bytes(size_t words) {
    return words * 8;
}
} // namespace MemoryLayout
