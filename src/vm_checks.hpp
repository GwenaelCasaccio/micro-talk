#pragma once
#include <cstdint>
#include <stdexcept>
#include <string>

#include "memory_layout.hpp"
#include "vm_exception.hpp"

// ============================================================================
// Bounds Checking Configuration
// ============================================================================
// Compile-time optional bounds checking for performance optimization.
// Default: enabled in debug builds, disabled in release builds (based on NDEBUG)
// Override: Define MICRO_TALK_BOUNDS_CHECKS=0 or =1 to force on/off
//
// Usage:
//   make                          - Debug build with bounds checks
//   make OPTIMIZE=1               - Release build without bounds checks
//   make BOUNDS_CHECKS=1          - Force enable bounds checks
//   make BOUNDS_CHECKS=0          - Force disable bounds checks
//   make OPTIMIZE=1 BOUNDS_CHECKS=1  - Release with checks (paranoid mode)
// ============================================================================
#ifndef MICRO_TALK_BOUNDS_CHECKS
#ifdef NDEBUG
#define MICRO_TALK_BOUNDS_CHECKS 0 // Release: disable for performance
#else
#define MICRO_TALK_BOUNDS_CHECKS 1 // Debug: enable for safety
#endif
#endif

// ============================================================================
// VM Bounds Checking Utilities
// ============================================================================
// Compile-time optional bounds checking for memory safety.
// Uses if constexpr for zero overhead when disabled.

namespace VMChecks {

// Compile-time flag for bounds checking (controlled by MICRO_TALK_BOUNDS_CHECKS macro)
static constexpr bool BOUNDS_CHECKS_ENABLED = (MICRO_TALK_BOUNDS_CHECKS != 0);

// Memory bounds checking (compile-time optional via if constexpr)
inline void check_memory_bounds(uint64_t addr, uint64_t ip, uint64_t sp, uint64_t bp, uint64_t hp) {
    if constexpr (BOUNDS_CHECKS_ENABLED) {
        if (addr >= MemoryLayout::MEMORY_SIZE) {
            throw VMException::MemoryBounds(addr, MemoryLayout::MEMORY_SIZE, ip, sp, bp, hp);
        }
    }
}

// IP bounds checking helper (compile-time optional via if constexpr)
inline void check_ip_bounds(uint64_t ip_val, uint64_t sp, uint64_t bp, uint64_t hp) {
    if constexpr (BOUNDS_CHECKS_ENABLED) {
        if (ip_val >= MemoryLayout::MEMORY_SIZE) {
            throw VMException::IPBounds(ip_val, MemoryLayout::MEMORY_SIZE, sp, bp, hp);
        }
    }
}

// Code segment write protection (compile-time optional via if constexpr)
inline void check_code_segment_protection(uint64_t addr, uint64_t ip, uint64_t sp, uint64_t bp,
                                          uint64_t hp) {
    if constexpr (BOUNDS_CHECKS_ENABLED) {
        if (addr < MemoryLayout::CODE_SIZE) {
            throw VMException::CodeSegmentProtection(addr, MemoryLayout::CODE_SIZE, ip, sp, bp, hp);
        }
    }
}

// Stack overflow detection (check before push)
inline void check_stack_overflow(uint64_t sp, uint64_t hp, uint64_t ip, uint64_t bp) {
    if constexpr (BOUNDS_CHECKS_ENABLED) {
        if (sp - 1 <= hp) {
            throw VMException::StackOverflow(sp, sp - 1, hp, ip, bp, hp);
        }
    }
}

// Stack underflow detection (check before pop)
inline void check_stack_underflow(uint64_t sp, uint64_t ip, uint64_t bp, uint64_t hp) {
    if constexpr (BOUNDS_CHECKS_ENABLED) {
        if (sp >= MemoryLayout::STACK_BASE) {
            throw VMException::StackUnderflow(sp, MemoryLayout::STACK_BASE, ip, bp, hp);
        }
    }
}

// Stack empty check (check before peek)
inline void check_stack_empty(uint64_t sp, uint64_t ip, uint64_t bp, uint64_t hp) {
    if constexpr (BOUNDS_CHECKS_ENABLED) {
        if (sp >= MemoryLayout::STACK_BASE) {
            throw VMException::StackEmpty(sp, MemoryLayout::STACK_BASE, ip, bp, hp);
        }
    }
}

// Stack frame boundary validation
inline void check_stack_frame_bounds(uint64_t addr, uint64_t sp, uint64_t ip, uint64_t bp,
                                     uint64_t hp) {
    if constexpr (BOUNDS_CHECKS_ENABLED) {
        if (addr < sp || addr >= MemoryLayout::STACK_BASE) {
            throw VMException::StackFrameBounds(addr, sp, MemoryLayout::STACK_BASE, ip, bp, hp);
        }
    }
}

// BP_STORE index validation (must be at least 1 to not overwrite return address)
inline void check_bp_store_index(uint64_t idx, uint64_t ip, uint64_t sp, uint64_t bp, uint64_t hp) {
    if constexpr (BOUNDS_CHECKS_ENABLED) {
        if (idx < 1) {
            throw VMException::InvalidBPStoreIndex(idx, ip, sp, bp, hp);
        }
    }
}

} // namespace VMChecks
