#pragma once
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>

// ============================================================================
// VM Exception System
// ============================================================================
// Custom exception hierarchy for VM runtime errors with structured context.
// All exceptions capture VM register state (IP, SP, BP, HP) for debugging.

namespace VMException {

// ============================================================================
// Base VM Exception
// ============================================================================
class Base : public std::runtime_error {
  protected:
    uint64_t ip_;
    uint64_t sp_;
    uint64_t bp_;
    uint64_t hp_;
    std::string category_;

    // Protected constructor for derived classes
    Base(const std::string& category, const std::string& message, uint64_t ip, uint64_t sp,
         uint64_t bp, uint64_t hp)
        : std::runtime_error(format_message(category, message, ip, sp, bp, hp)), ip_(ip), sp_(sp),
          bp_(bp), hp_(hp), category_(category) {}

    // Format error message with VM state
    static std::string format_message(const std::string& category, const std::string& message,
                                      uint64_t ip, uint64_t sp, uint64_t bp, uint64_t hp) {
        std::ostringstream oss;
        oss << "[" << category << "] " << message << " (IP=" << ip << ", SP=" << sp << ", BP=" << bp
            << ", HP=" << hp << ")";
        return oss.str();
    }

  public:
    // Accessors for VM state
    [[nodiscard]] uint64_t get_ip() const {
        return ip_;
    }
    [[nodiscard]] uint64_t get_sp() const {
        return sp_;
    }
    [[nodiscard]] uint64_t get_bp() const {
        return bp_;
    }
    [[nodiscard]] uint64_t get_hp() const {
        return hp_;
    }
    [[nodiscard]] const std::string& get_category() const {
        return category_;
    }
};

// ============================================================================
// Memory Access Exceptions
// ============================================================================
class MemoryBounds : public Base {
  private:
    uint64_t address_;
    uint64_t memory_size_;

  public:
    MemoryBounds(uint64_t address, uint64_t memory_size, uint64_t ip, uint64_t sp, uint64_t bp,
                 uint64_t hp)
        : Base("MEMORY_BOUNDS",
               "Memory access out of bounds: address " + std::to_string(address) +
                   " exceeds memory size " + std::to_string(memory_size),
               ip, sp, bp, hp),
          address_(address), memory_size_(memory_size) {}

    [[nodiscard]] uint64_t get_address() const {
        return address_;
    }
    [[nodiscard]] uint64_t get_memory_size() const {
        return memory_size_;
    }
};

class CodeSegmentProtection : public Base {
  private:
    uint64_t address_;
    uint64_t code_size_;

  public:
    CodeSegmentProtection(uint64_t address, uint64_t code_size, uint64_t ip, uint64_t sp,
                          uint64_t bp, uint64_t hp)
        : Base("CODE_PROTECTION",
               "Cannot write to code segment: address " + std::to_string(address) +
                   " is in protected region [0, " + std::to_string(code_size) + ")",
               ip, sp, bp, hp),
          address_(address), code_size_(code_size) {}

    [[nodiscard]] uint64_t get_address() const {
        return address_;
    }
    [[nodiscard]] uint64_t get_code_size() const {
        return code_size_;
    }
};

class IPBounds : public Base {
  private:
    uint64_t ip_value_;
    uint64_t memory_size_;

  public:
    IPBounds(uint64_t ip_value, uint64_t memory_size, uint64_t sp, uint64_t bp, uint64_t hp)
        : Base("IP_BOUNDS",
               "Instruction pointer out of bounds: IP " + std::to_string(ip_value) +
                   " exceeds memory size " + std::to_string(memory_size),
               ip_value, sp, bp, hp),
          ip_value_(ip_value), memory_size_(memory_size) {}

    [[nodiscard]] uint64_t get_ip_value() const {
        return ip_value_;
    }
    [[nodiscard]] uint64_t get_memory_size() const {
        return memory_size_;
    }
};

// ============================================================================
// Stack Exceptions
// ============================================================================
class StackOverflow : public Base {
  private:
    uint64_t next_sp_;
    uint64_t hp_value_;

  public:
    StackOverflow(uint64_t sp, uint64_t next_sp, uint64_t hp_value, uint64_t ip, uint64_t bp,
                  uint64_t hp)
        : Base("STACK_OVERFLOW",
               "Stack overflow: next SP " + std::to_string(next_sp) +
                   " would collide with heap at " + std::to_string(hp_value),
               ip, sp, bp, hp),
          next_sp_(next_sp), hp_value_(hp_value) {}

    [[nodiscard]] uint64_t get_next_sp() const {
        return next_sp_;
    }
    [[nodiscard]] uint64_t get_hp_value() const {
        return hp_value_;
    }
};

class StackUnderflow : public Base {
  private:
    uint64_t stack_base_;

  public:
    StackUnderflow(uint64_t sp, uint64_t stack_base, uint64_t ip, uint64_t bp, uint64_t hp)
        : Base("STACK_UNDERFLOW",
               "Stack underflow: SP " + std::to_string(sp) + " at or above stack base " +
                   std::to_string(stack_base),
               ip, sp, bp, hp),
          stack_base_(stack_base) {}

    [[nodiscard]] uint64_t get_stack_base() const {
        return stack_base_;
    }
};

class StackEmpty : public Base {
  private:
    uint64_t stack_base_;

  public:
    StackEmpty(uint64_t sp, uint64_t stack_base, uint64_t ip, uint64_t bp, uint64_t hp)
        : Base("STACK_EMPTY",
               "Stack is empty: SP " + std::to_string(sp) + " at stack base " +
                   std::to_string(stack_base),
               ip, sp, bp, hp),
          stack_base_(stack_base) {}

    [[nodiscard]] uint64_t get_stack_base() const {
        return stack_base_;
    }
};

class StackFrameBounds : public Base {
  private:
    uint64_t address_;
    uint64_t frame_start_;
    uint64_t frame_end_;

  public:
    StackFrameBounds(uint64_t address, uint64_t sp, uint64_t stack_base, uint64_t ip, uint64_t bp,
                     uint64_t hp)
        : Base("STACK_FRAME_BOUNDS",
               "Access outside stack frame: address " + std::to_string(address) +
                   " not in range [" + std::to_string(sp) + ", " + std::to_string(stack_base) + ")",
               ip, sp, bp, hp),
          address_(address), frame_start_(sp), frame_end_(stack_base) {}

    [[nodiscard]] uint64_t get_address() const {
        return address_;
    }
    [[nodiscard]] uint64_t get_frame_start() const {
        return frame_start_;
    }
    [[nodiscard]] uint64_t get_frame_end() const {
        return frame_end_;
    }
};

class InvalidBPStoreIndex : public Base {
  private:
    uint64_t index_;

  public:
    InvalidBPStoreIndex(uint64_t index, uint64_t ip, uint64_t sp, uint64_t bp, uint64_t hp)
        : Base("INVALID_BP_STORE",
               "Invalid BP_STORE index " + std::to_string(index) +
                   ": must be at least 1 to avoid overwriting return address",
               ip, sp, bp, hp),
          index_(index) {}

    [[nodiscard]] uint64_t get_index() const {
        return index_;
    }
};

// ============================================================================
// Execution Exceptions
// ============================================================================
class DivisionByZero : public Base {
  private:
    std::string operation_;

  public:
    DivisionByZero(const std::string& operation, uint64_t ip, uint64_t sp, uint64_t bp, uint64_t hp)
        : Base("DIVISION_BY_ZERO", operation + " by zero", ip, sp, bp, hp), operation_(operation) {}

    [[nodiscard]] const std::string& get_operation() const {
        return operation_;
    }
};

class UnknownOpcode : public Base {
  private:
    uint64_t opcode_;

  public:
    UnknownOpcode(uint64_t opcode, uint64_t ip, uint64_t sp, uint64_t bp, uint64_t hp)
        : Base("UNKNOWN_OPCODE", "Unknown opcode: " + std::to_string(opcode), ip, sp, bp, hp),
          opcode_(opcode) {}

    [[nodiscard]] uint64_t get_opcode() const {
        return opcode_;
    }
};

class InvalidSignal : public Base {
  private:
    uint64_t signal_;
    int min_signal_;
    int max_signal_;

  public:
    InvalidSignal(uint64_t signal, int min_signal, int max_signal, uint64_t ip, uint64_t sp,
                  uint64_t bp, uint64_t hp)
        : Base("INVALID_SIGNAL",
               "Invalid signal " + std::to_string(signal) + ": must be in range [" +
                   std::to_string(min_signal) + ", " + std::to_string(max_signal) + "]",
               ip, sp, bp, hp),
          signal_(signal), min_signal_(min_signal), max_signal_(max_signal) {}

    [[nodiscard]] uint64_t get_signal() const {
        return signal_;
    }
    [[nodiscard]] int get_min_signal() const {
        return min_signal_;
    }
    [[nodiscard]] int get_max_signal() const {
        return max_signal_;
    }
};

class InvalidAddress : public Base {
  private:
    uint64_t address_;
    std::string operation_;

  public:
    InvalidAddress(const std::string& operation, uint64_t address, uint64_t ip, uint64_t sp,
                   uint64_t bp, uint64_t hp)
        : Base("INVALID_ADDRESS", operation + " address out of bounds: " + std::to_string(address),
               ip, sp, bp, hp),
          address_(address), operation_(operation) {}

    [[nodiscard]] uint64_t get_address() const {
        return address_;
    }
    [[nodiscard]] const std::string& get_operation() const {
        return operation_;
    }
};

// ============================================================================
// Snapshot/Restore Exceptions
// ============================================================================
class InvalidSnapshot : public std::runtime_error {
  private:
    size_t expected_size_;
    size_t actual_size_;

  public:
    InvalidSnapshot(size_t expected_size, size_t actual_size)
        : std::runtime_error("Invalid snapshot: expected memory size " +
                             std::to_string(expected_size) + " but got " +
                             std::to_string(actual_size)),
          expected_size_(expected_size), actual_size_(actual_size) {}

    [[nodiscard]] size_t get_expected_size() const {
        return expected_size_;
    }
    [[nodiscard]] size_t get_actual_size() const {
        return actual_size_;
    }
};

// ============================================================================
// Program Loading Exceptions
// ============================================================================
class ProgramTooLarge : public std::runtime_error {
  private:
    size_t program_size_;
    size_t code_size_;

  public:
    ProgramTooLarge(size_t program_size, size_t code_size)
        : std::runtime_error("Program too large: " + std::to_string(program_size) +
                             " words exceeds code segment size of " + std::to_string(code_size) +
                             " words"),
          program_size_(program_size), code_size_(code_size) {}

    [[nodiscard]] size_t get_program_size() const {
        return program_size_;
    }
    [[nodiscard]] size_t get_code_size() const {
        return code_size_;
    }
};

class MemoryAllocationFailed : public std::runtime_error {
  public:
    MemoryAllocationFailed() : std::runtime_error("Failed to allocate VM memory using mmap") {}
};

} // namespace VMException
