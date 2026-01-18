#pragma once
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <sys/mman.h>
#include <unistd.h>
#include <utility>
#include <vector>

#include "compiled_program.hpp"
#include "eval_context.hpp"
#include "interrupt.hpp"
#include "memory_layout.hpp"
#include "opcodes.hpp"
#include "vm_checks.hpp"
#include "vm_exception.hpp"

// ============================================================================
// Stack Virtual Machine
// ============================================================================
// A 64-bit stack-based virtual machine with flat memory model and
// computed goto dispatch for high-performance bytecode execution.

class StackVM {
  private:
    // Memory layout constants are defined in memory_layout.hpp
    // See MemoryLayout namespace for region boundaries and helper functions
    static constexpr size_t CODE_SIZE = MemoryLayout::CODE_SIZE;
    static constexpr size_t GLOBALS_START = MemoryLayout::GLOBALS_START;
    static constexpr size_t HEAP_START = MemoryLayout::HEAP_START;
    static constexpr size_t STACK_START = MemoryLayout::STACK_START;
    static constexpr size_t MEMORY_SIZE = MemoryLayout::MEMORY_SIZE;
    static constexpr size_t STACK_BASE = MemoryLayout::STACK_BASE;

    // Signal handling constants
    static constexpr int MIN_SIGNAL = 1;
    static constexpr int MAX_SIGNAL = 31;
    static constexpr size_t NUM_SIGNALS = MAX_SIGNAL;

    // String packing constants
    static constexpr size_t BYTES_PER_WORD = 8;

    InterruptHandling interruptHandling;
    bool interrupt_flag{true};
    std::array<uint64_t, NUM_SIGNALS> signal_handlers{0};

    uint64_t* memory; // Flat linear mmap'd memory (64-bit words)
    uint64_t ip{0};   // Instruction pointer
    uint64_t sp;      // Stack pointer
    uint64_t bp;      // Base/frame pointer
    uint64_t hp;      // Heap pointer
    bool running{false};
    bool trace_mode{false};            // Enable trace/debug output during execution
    bool hit_instruction_limit{false}; // True if execution stopped due to instruction limit

    // Execution profiling support
    bool profiling_enabled{false};
    std::array<uint64_t, 256> opcode_counts{0}; // Count execution of each opcode
    uint64_t total_instructions{0};             // Total instructions executed

    // Runtime code generation support (for EVAL and COMPILE opcodes)
    EvalContext* eval_ctx{nullptr};

    // Helper method to read packed strings from memory
    std::string read_packed_string(uint64_t addr) const {
        VMChecks::check_memory_bounds(addr, ip, sp, bp, hp);
        const uint64_t length = memory[addr];
        std::string result;
        result.reserve(length);

        for (size_t i = 0; i < length; i++) {
            const size_t word_idx = addr + 1 + i / BYTES_PER_WORD;
            const size_t byte_idx = i % BYTES_PER_WORD;
            const char c = static_cast<char>((memory[word_idx] >> (byte_idx * 8)) & 0xFF);
            result += c;
        }

        return result;
    }

    inline void push(uint64_t value) {
        VMChecks::check_stack_overflow(sp, hp, ip, bp);
        memory[--sp] = value;
    }

    inline uint64_t pop() {
        VMChecks::check_stack_underflow(sp, ip, bp, hp);
        return memory[sp++];
    }

    [[nodiscard]] inline uint64_t peek() const {
        VMChecks::check_stack_empty(sp, ip, bp, hp);
        return memory[sp];
    }

  public:
    StackVM() : sp(STACK_BASE), bp(STACK_BASE), hp(HEAP_START) {
        // Allocate memory using mmap
        void* ptr = mmap(nullptr, MEMORY_SIZE * sizeof(uint64_t), PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        if (ptr == MAP_FAILED) {
            throw VMException::MemoryAllocationFailed();
        }

        memory = static_cast<uint64_t*>(ptr);
        reset();
    }

    ~StackVM() {
        if (memory != nullptr) {
            munmap(memory, MEMORY_SIZE * sizeof(uint64_t));
        }
    }

    // Delete copy constructor and assignment
    StackVM(const StackVM&) = delete;
    StackVM& operator=(const StackVM&) = delete;

    // Load bytecode into code segment starting at address 0
    void load_program(const std::vector<uint64_t>& program) {
        if (program.size() > CODE_SIZE) {
            throw VMException::ProgramTooLarge(program.size(), CODE_SIZE);
        }
        memcpy(memory, program.data(), program.size() * sizeof(uint64_t));
    }

    // Load bytecode at a specific offset (for REPL to append code)
    void load_program_at(const std::vector<uint64_t>& program, uint64_t offset) {
        if (offset + program.size() > CODE_SIZE) {
            throw VMException::ProgramTooLarge(offset + program.size(), CODE_SIZE);
        }
        memcpy(memory + offset, program.data(), program.size() * sizeof(uint64_t));
    }

    // Load compiled program (bytecode + data)
    void load_program(const CompiledProgram& program) {
        // Load bytecode
        if (program.bytecode.size() > CODE_SIZE) {
            throw VMException::ProgramTooLarge(program.bytecode.size(), CODE_SIZE);
        }
        memcpy(memory, program.bytecode.data(), program.bytecode.size() * sizeof(uint64_t));

        // Write string literals to memory
        program.write_strings(*this);
    }

    // Load compiled program at a specific offset (for REPL)
    void load_program_at(const CompiledProgram& program, uint64_t offset) {
        if (offset + program.bytecode.size() > CODE_SIZE) {
            throw VMException::ProgramTooLarge(offset + program.bytecode.size(), CODE_SIZE);
        }
        memcpy(memory + offset, program.bytecode.data(),
               program.bytecode.size() * sizeof(uint64_t));

        // Write string literals to memory
        program.write_strings(*this);
    }

    void reset() {
        ip = 0;
        sp = STACK_BASE;
        bp = STACK_BASE;
        running = true;
        hit_instruction_limit = false;
        interruptHandling.clear();
    }

    // Set instruction pointer (for REPL to start execution at specific address)
    void set_ip(uint64_t new_ip) {
        ip = new_ip;
    }

    // Execute the loaded program
    // max_instructions: Maximum number of instructions to execute (default: unlimited)
    //                   Provides protection against infinite loops
    void execute(uint64_t max_instructions = UINT64_MAX) {
        uint64_t instruction_count = 0;
        while (running && ip < MEMORY_SIZE && instruction_count < max_instructions) {
            instruction_count++;
            if (interrupt_flag && interruptHandling.has_event()) {
                for (int sig = MIN_SIGNAL; sig <= MAX_SIGNAL; sig++) {
                    if (interruptHandling.get_count(sig) > 0 &&
                        signal_handlers[sig - MIN_SIGNAL] != 0) {
                        interrupt_flag = false;
                        push(ip);
                        ip = signal_handlers[sig - MIN_SIGNAL];
                        interruptHandling.consumme(sig);
                        break;
                    }
                }
            }

            const uint64_t instruction = memory[ip++];
            const auto op = static_cast<Opcode>(instruction & 0xFF);

            if (trace_mode) {
                std::cerr << "IP=" << (ip - 1) << " OP=" << opcode_name(op) << " SP=" << sp
                          << " BP=" << bp << " HP=" << hp << std::endl;
            }

            // Computed goto dispatch table for faster branch prediction
            static const void* dispatch_table[] = {
                &&op_halt,       &&op_push,      &&op_pop,     &&op_dup,     &&op_swap,
                &&op_add,        &&op_sub,       &&op_mul,     &&op_div,     &&op_mod,
                &&op_eq,         &&op_lt,        &&op_gt,      &&op_lte,     &&op_gte,
                &&op_jmp,        &&op_jz,        &&op_enter,   &&op_leave,   &&op_call,
                &&op_ret,        &&op_iret,      &&op_load,    &&op_store,   &&op_load_byte,
                &&op_store_byte, &&op_load32,    &&op_store32, &&op_bp_load, &&op_bp_store,
                &&op_print,      &&op_print_str, &&op_and,     &&op_or,      &&op_xor,
                &&op_shl,        &&op_shr,       &&op_ashr,    &&op_cli,     &&op_sti,
                &&op_signal_reg, &&op_abort,     &&op_funcall, &&op_eval,    &&op_compile};

            const uint8_t opcode_idx = static_cast<uint8_t>(op);
            if (opcode_idx >= 45) {
                throw VMException::UnknownOpcode(opcode_idx, ip - 1, sp, bp, hp);
            }

            // Update profiling counters if enabled
            if (profiling_enabled) {
                opcode_counts[opcode_idx]++;
                total_instructions++;
            }

            goto* dispatch_table[opcode_idx];

        op_halt:
            running = false;
            continue;

        op_push:
            VMChecks::check_ip_bounds(ip, sp, bp, hp);
            push(memory[ip++]);
            continue;

        op_pop:
            pop();
            continue;

        op_dup:
            push(peek());
            continue;

        op_swap: {
            uint64_t a = pop();
            uint64_t b = pop();
            push(a);
            push(b);
            continue;
        }

        op_add: {
            uint64_t b = pop();
            uint64_t a = pop();
            push(a + b);
            continue;
        }

        op_sub: {
            uint64_t b = pop();
            uint64_t a = pop();
            push(a - b);
            continue;
        }

        op_mul: {
            uint64_t b = pop();
            uint64_t a = pop();
            push(a * b);
            continue;
        }

        op_div: {
            uint64_t b = pop();
            uint64_t a = pop();
            if (b == 0)
                throw VMException::DivisionByZero("Division", ip - 1, sp, bp, hp);
            push(a / b);
            continue;
        }

        op_mod: {
            uint64_t b = pop();
            uint64_t a = pop();
            if (b == 0)
                throw VMException::DivisionByZero("Modulo", ip - 1, sp, bp, hp);
            push(a % b);
            continue;
        }

        op_eq: {
            uint64_t b = pop();
            uint64_t a = pop();
            push(a == b ? 1 : 0);
            continue;
        }

        op_lt: {
            int64_t b = static_cast<int64_t>(pop());
            int64_t a = static_cast<int64_t>(pop());
            push(a < b ? 1 : 0);
            continue;
        }

        op_gt: {
            int64_t b = static_cast<int64_t>(pop());
            int64_t a = static_cast<int64_t>(pop());
            push(a > b ? 1 : 0);
            continue;
        }

        op_lte: {
            int64_t b = static_cast<int64_t>(pop());
            int64_t a = static_cast<int64_t>(pop());
            push(a <= b ? 1 : 0);
            continue;
        }

        op_gte: {
            int64_t b = static_cast<int64_t>(pop());
            int64_t a = static_cast<int64_t>(pop());
            push(a >= b ? 1 : 0);
            continue;
        }

        op_jmp:
            VMChecks::check_ip_bounds(ip, sp, bp, hp);
            ip = memory[ip];
            VMChecks::check_ip_bounds(ip, sp, bp, hp);
            continue;

        op_jz: {
            VMChecks::check_ip_bounds(ip, sp, bp, hp);
            uint64_t cond = pop();
            uint64_t addr = memory[ip++];
            if (cond == 0) {
                ip = addr;
                VMChecks::check_ip_bounds(ip, sp, bp, hp);
            }
            continue;
        }

        op_enter: {
            VMChecks::check_ip_bounds(ip, sp, bp, hp);
            const size_t temp_size = memory[ip++];

            for (uint64_t i = 0; i < temp_size; i++) {
                push(0);
            }
            push(bp);
            bp = sp;

            continue;
        }

        op_leave: {
            VMChecks::check_ip_bounds(ip, sp, bp, hp);
            const size_t temp_size = memory[ip++];
            const uint64_t result = pop();

            bp = pop();

            for (uint64_t i = 0; i < temp_size; i++) {
                pop();
            }

            push(result);
            continue;
        }

        op_call: {
            VMChecks::check_ip_bounds(ip, sp, bp, hp);
            const uint64_t target = memory[ip++]; // Read target and advance IP
            VMChecks::check_ip_bounds(ip, sp, bp, hp);
            const uint64_t nb_args = memory[ip++]; // Arguments

            const uint64_t base = sp;

            push(ip);    // Save return address (now past the operand)
            ip = target; // Jump to function

            for (uint64_t i = nb_args; i > 0; i--) {
                push(memory[base + nb_args - i]);
            }

            VMChecks::check_ip_bounds(ip, sp, bp, hp);
            continue;
        }

        op_ret: {
            VMChecks::check_ip_bounds(ip, sp, bp, hp);
            const size_t nb_args = memory[ip++];

            const uint64_t result = pop();

            for (uint64_t i = 0; i < nb_args; i++) {
                pop();
            }

            const uint64_t ret_addr = pop(); // Pop return address

            for (uint64_t i = 0; i < nb_args; i++) {
                pop();
            }

            push(result);

            ip = ret_addr; // Return to caller

            VMChecks::check_ip_bounds(ip, sp, bp, hp);

            continue;
        }

        op_iret: {
            ip = pop();

            VMChecks::check_ip_bounds(ip, sp, bp, hp);

            interrupt_flag = true;

            continue;
        }

        op_load: {
            uint64_t addr = pop();
            VMChecks::check_memory_bounds(addr, ip, sp, bp, hp);
            push(memory[addr]);
            continue;
        }

        op_store: {
            uint64_t addr = pop();
            uint64_t value = pop();
            VMChecks::check_memory_bounds(addr, ip, sp, bp, hp);
            VMChecks::check_code_segment_protection(addr, ip, sp, bp, hp);
            memory[addr] = value;
            continue;
        }

        op_load_byte: {
            // Load a single byte from memory
            // Stack: [addr] -> [byte_value]
            uint64_t addr = pop();
            VMChecks::check_memory_bounds(addr / 8, ip, sp, bp, hp); // Check word boundary

            // Calculate word index and byte offset
            uint64_t word_idx = addr / 8;
            uint64_t byte_offset = addr % 8;

            // Extract byte
            uint64_t word = memory[word_idx];
            uint8_t byte_val = (word >> (byte_offset * 8)) & 0xFF;

            push(byte_val);
            continue;
        }

        op_store_byte: {
            // Store a single byte to memory
            // Stack: [value, addr] -> []
            uint64_t addr = pop();
            uint64_t value = pop() & 0xFF; // Only keep low byte

            VMChecks::check_memory_bounds(addr / 8, ip, sp, bp, hp);
            uint64_t word_idx = addr / 8;

            VMChecks::check_code_segment_protection(word_idx, ip, sp, bp, hp);

            uint64_t byte_offset = addr % 8;

            // Read-modify-write: clear byte, then set new value
            uint64_t mask = ~(0xFFULL << (byte_offset * 8));
            memory[word_idx] = (memory[word_idx] & mask) | (value << (byte_offset * 8));
            continue;
        }

        op_load32: {
            // Load a 32-bit word from memory
            // Stack: [addr] -> [32bit_value]
            uint64_t addr = pop();
            VMChecks::check_memory_bounds(addr / 8, ip, sp, bp, hp);

            uint64_t word_idx = addr / 8;
            uint64_t is_high = (addr / 4) % 2; // 0 = low 32 bits, 1 = high 32 bits

            uint64_t word = memory[word_idx];
            uint32_t val32 = is_high ? (word >> 32) : (word & 0xFFFFFFFF);

            push(val32);
            continue;
        }

        op_store32: {
            // Store a 32-bit word to memory
            // Stack: [value, addr] -> []
            uint64_t addr = pop();
            uint64_t value = pop() & 0xFFFFFFFF; // Only keep low 32 bits

            VMChecks::check_memory_bounds(addr / 8, ip, sp, bp, hp);
            uint64_t word_idx = addr / 8;

            VMChecks::check_code_segment_protection(word_idx, ip, sp, bp, hp);

            uint64_t is_high = (addr / 4) % 2;

            if (is_high) {
                // Store in high 32 bits
                memory[word_idx] = (memory[word_idx] & 0xFFFFFFFF) | (value << 32);
            } else {
                // Store in low 32 bits
                memory[word_idx] = (memory[word_idx] & 0xFFFFFFFF00000000ULL) | value;
            }
            continue;
        }

        op_bp_load: {
            const uint64_t idx = pop();
            const uint64_t adr = bp + idx + 1;
            VMChecks::check_memory_bounds(adr, ip, sp, bp, hp);
            VMChecks::check_stack_frame_bounds(adr, sp, ip, bp, hp);
            const uint64_t value = memory[adr];
            push(value);
            continue;
        }

        op_bp_store: {
            const uint64_t idx = pop();
            const uint64_t value = pop();
            const uint64_t adr = bp + idx + 1; // with SP ++ IP on the stack
            VMChecks::check_memory_bounds(adr, ip, sp, bp, hp);
            VMChecks::check_bp_store_index(idx, ip, sp, bp, hp);
            VMChecks::check_stack_frame_bounds(adr, sp, ip, bp, hp);
            VMChecks::check_code_segment_protection(adr, ip, sp, bp, hp);
            memory[adr] = value;
            continue;
        }

        op_print:
            std::cout << "DEBUG: " << peek() << '\n';
            continue;

        op_print_str: {
            uint64_t addr = peek();
            std::string str = read_packed_string(addr);
            std::cout << "DEBUG_STR: " << str << '\n';
            continue;
        }

        op_and: {
            uint64_t b = pop();
            uint64_t a = pop();
            push(a & b);
            continue;
        }

        op_or: {
            uint64_t b = pop();
            uint64_t a = pop();
            push(a | b);
            continue;
        }

        op_xor: {
            uint64_t b = pop();
            uint64_t a = pop();
            push(a ^ b);
            continue;
        }

        op_shl: {
            uint64_t b = pop();
            uint64_t a = pop();
            push(a << b);
            continue;
        }

        op_shr: {
            uint64_t b = pop();
            uint64_t a = pop();
            push(a >> b);
            continue;
        }

        op_ashr: {
            uint64_t b = pop();
            uint64_t a = pop();
            auto signed_a = static_cast<int64_t>(a);
            push(static_cast<uint64_t>(signed_a >> b));
            continue;
        }

        op_cli:
            interrupt_flag = false;
            continue;

        op_sti:
            interrupt_flag = true;
            continue;

        op_signal_reg: {
            const uint64_t signal = pop();
            const size_t code_ptr = pop();

            if (signal < MIN_SIGNAL || signal > MAX_SIGNAL)
                throw VMException::InvalidSignal(signal, MIN_SIGNAL, MAX_SIGNAL, ip - 1, sp, bp,
                                                 hp);

            if (code_ptr >= MEMORY_SIZE) {
                throw VMException::InvalidAddress("Interrupt handler", code_ptr, ip - 1, sp, bp,
                                                  hp);
            }

            signal_handlers[signal - MIN_SIGNAL] = code_ptr;

            continue;
        }

        op_abort: {
            const uint64_t addr = pop();
            if (addr >= MEMORY_SIZE) {
                std::cerr << "ABORT: Invalid string address " << addr << std::endl;
                running = false;
                continue;
            }

            // Read string using helper method
            std::string message = read_packed_string(addr);
            std::cout << "ABORT: " << message << std::endl;
            running = false;
            continue;
        }

        op_funcall: {
            // Stack: [arg1] [arg2] ... [argN] [arg_count] [target_address]
            const uint64_t target = pop();
            const uint64_t nb_args = pop();

            if (target >= MEMORY_SIZE)
                throw VMException::InvalidAddress("FUNCALL target", target, ip - 1, sp, bp, hp);

            const uint64_t base = sp;

            push(ip);    // Save return address
            ip = target; // Jump to function

            // Push arguments to new frame
            for (uint64_t i = nb_args; i > 0; i--) {
                push(memory[base + nb_args - i]);
            }

            continue;
        }

        op_eval: {
            // EVAL: Pop string addr, compile, execute inline, push result
            // Stack: [string_addr] -> [result]
            if (!eval_ctx || !eval_ctx->compile_for_eval) {
                throw std::runtime_error("EVAL: no eval context configured");
            }

            uint64_t str_addr = pop();
            std::string code = read_packed_string(str_addr);

            // Compile the code (ends with HALT)
            uint64_t code_addr = eval_ctx->compile_for_eval(*this, code);

            // Save current IP as return address
            push(ip);

            // Jump to compiled code
            // Note: The compiled code ends with HALT, which will stop execution.
            // The caller should handle this by saving/restoring state or use
            // a recursive execute call.

            // We use a different approach: compile code ending with RET 0 for eval
            // and then call it like a function with no arguments
            ip = code_addr;

            continue;
        }

        op_compile: {
            // COMPILE: Pop string addr, compile, push code address
            // Stack: [string_addr] -> [code_addr]
            if (!eval_ctx || !eval_ctx->compile_for_funcall) {
                throw std::runtime_error("COMPILE: no eval context configured");
            }

            uint64_t str_addr = pop();
            std::string code = read_packed_string(str_addr);

            // Compile the code (ends with RET for funcall)
            uint64_t code_addr = eval_ctx->compile_for_funcall(*this, code);

            // Push the code address - caller can use funcall to invoke it
            push(code_addr);

            continue;
        }
        }

        // Check if we stopped due to instruction limit
        hit_instruction_limit = (instruction_count >= max_instructions && running);
        running = true;
    }

    // Debug accessors
    [[nodiscard]] uint64_t get_top() const {
        return peek();
    }
    uint64_t stack_pop() {
        return pop();
    }
    [[nodiscard]] uint64_t get_ip() const {
        return ip;
    }
    [[nodiscard]] uint64_t get_sp() const {
        return sp;
    }
    [[nodiscard]] uint64_t get_bp() const {
        return bp;
    }
    InterruptHandling& get_interrupt_handling() {
        return interruptHandling;
    }
    bool interrupts_enabled() {
        return interrupt_flag;
    }
    uint64_t get_signal_handler(int signal) {
        if (signal < MIN_SIGNAL || signal > MAX_SIGNAL)
            throw VMException::InvalidSignal(signal, MIN_SIGNAL, MAX_SIGNAL, ip, sp, bp, hp);
        return signal_handlers[signal - MIN_SIGNAL];
    }

    [[nodiscard]] uint64_t read_memory(uint64_t addr) const {
        VMChecks::check_memory_bounds(addr, ip, sp, bp, hp);
        return memory[addr];
    }
    void write_memory(uint64_t addr, uint64_t value) {
        VMChecks::check_memory_bounds(addr, ip, sp, bp, hp);
        if (addr < CODE_SIZE) {
            throw VMException::CodeSegmentProtection(addr, CODE_SIZE, ip, sp, bp, hp);
        }
        memory[addr] = value;
    }

    // Debug/trace mode controls
    void set_trace_mode(bool enabled) {
        trace_mode = enabled;
    }
    [[nodiscard]] bool get_trace_mode() const {
        return trace_mode;
    }

    // Eval context for runtime code generation
    void set_eval_context(EvalContext* ctx) {
        eval_ctx = ctx;
    }
    [[nodiscard]] EvalContext* get_eval_context() const {
        return eval_ctx;
    }

    // Execution limit control
    [[nodiscard]] bool hit_instruction_limit_check() const {
        return hit_instruction_limit;
    }

    // Execution profiling controls
    void enable_profiling() {
        profiling_enabled = true;
    }
    void disable_profiling() {
        profiling_enabled = false;
    }
    [[nodiscard]] bool is_profiling_enabled() const {
        return profiling_enabled;
    }

    // Reset profiling counters
    void reset_profiling() {
        opcode_counts.fill(0);
        total_instructions = 0;
    }

    // Get profiling statistics
    [[nodiscard]] uint64_t get_total_instructions() const {
        return total_instructions;
    }
    [[nodiscard]] uint64_t get_opcode_count(Opcode op) const {
        return opcode_counts[static_cast<uint8_t>(op)];
    }
    [[nodiscard]] const std::array<uint64_t, 256>& get_opcode_counts() const {
        return opcode_counts;
    }

    // Print profiling report
    void print_profiling_report() const {
        if (total_instructions == 0) {
            std::cerr
                << "No profiling data available (profiling may be disabled or no instructions "
                   "executed)"
                << std::endl;
            return;
        }

        std::cerr << "\n=== VM Execution Profile ===" << std::endl;
        std::cerr << "Total instructions executed: " << total_instructions << std::endl;
        std::cerr << "\nOpcode execution counts (sorted by frequency):" << std::endl;

        // Create vector of (opcode, count) pairs for sorting
        std::vector<std::pair<uint8_t, uint64_t>> sorted_opcodes;
        for (uint8_t i = 0; i < 43; i++) { // Only include valid opcodes
            if (opcode_counts[i] > 0) {
                sorted_opcodes.emplace_back(i, opcode_counts[i]);
            }
        }

        // Sort by count (descending)
        std::sort(sorted_opcodes.begin(), sorted_opcodes.end(),
                  [](const auto& a, const auto& b) { return a.second > b.second; });

        // Print sorted results
        for (const auto& [opcode_idx, count] : sorted_opcodes) {
            auto op = static_cast<Opcode>(opcode_idx);
            double percentage = (100.0 * count) / total_instructions;
            std::cerr << "  " << std::setw(15) << std::left << opcode_name(op) << ": "
                      << std::setw(12) << std::right << count << " (" << std::fixed
                      << std::setprecision(2) << percentage << "%)" << std::endl;
        }
        std::cerr << "========================" << std::endl;
    }

    // VM state snapshot for save/restore
    struct VMSnapshot {
        uint64_t ip, sp, bp, hp;
        std::vector<uint64_t> memory;
        bool interrupt_flag;
        std::array<uint64_t, NUM_SIGNALS> signal_handlers;
        bool running;
        bool trace_mode;
        bool profiling_enabled;
        std::array<uint64_t, 256> opcode_counts;
        uint64_t total_instructions;
    };

    // Checkpoint: Save current VM state
    VMSnapshot checkpoint() const {
        VMSnapshot snap;
        snap.ip = ip;
        snap.sp = sp;
        snap.bp = bp;
        snap.hp = hp;
        snap.interrupt_flag = interrupt_flag;
        snap.signal_handlers = signal_handlers;
        snap.running = running;
        snap.trace_mode = trace_mode;
        snap.profiling_enabled = profiling_enabled;
        snap.opcode_counts = opcode_counts;
        snap.total_instructions = total_instructions;

        // Copy entire memory
        snap.memory.resize(MEMORY_SIZE);
        memcpy(snap.memory.data(), memory, MEMORY_SIZE * sizeof(uint64_t));

        return snap;
    }

    // Restore: Restore VM to a previously saved state
    void restore(const VMSnapshot& snap) {
        if (snap.memory.size() != MEMORY_SIZE) {
            throw VMException::InvalidSnapshot(MEMORY_SIZE, snap.memory.size());
        }

        ip = snap.ip;
        sp = snap.sp;
        bp = snap.bp;
        hp = snap.hp;
        interrupt_flag = snap.interrupt_flag;
        signal_handlers = snap.signal_handlers;
        running = snap.running;
        trace_mode = snap.trace_mode;
        profiling_enabled = snap.profiling_enabled;
        opcode_counts = snap.opcode_counts;
        total_instructions = snap.total_instructions;

        // Restore entire memory
        memcpy(memory, snap.memory.data(), MEMORY_SIZE * sizeof(uint64_t));
    }

    // Dump stack contents (top 'count' elements)
    void dump_stack(size_t count = 10) const {
        std::cerr << "=== Stack Dump ===" << std::endl;
        std::cerr << "SP=" << sp << " (STACK_BASE=" << STACK_BASE << ")" << std::endl;

        size_t depth = STACK_BASE - sp;
        size_t to_print = std::min(count, depth);

        for (size_t i = 0; i < to_print; i++) {
            uint64_t addr = sp + i;
            std::cerr << "  [SP+" << i << "] @" << addr << ": " << memory[addr] << " (0x"
                      << std::hex << memory[addr] << std::dec << ")" << std::endl;
        }

        if (depth > to_print) {
            std::cerr << "  ... (" << (depth - to_print) << " more)" << std::endl;
        }
    }

    // Dump memory range [start, end)
    void dump_memory(uint64_t start, uint64_t end) const {
        std::cerr << "=== Memory Dump [" << start << ", " << end << ") ===" << std::endl;

        if (end > MEMORY_SIZE) {
            end = MEMORY_SIZE;
        }
        if (start >= end) {
            std::cerr << "Invalid range" << std::endl;
            return;
        }

        for (uint64_t addr = start; addr < end; addr++) {
            std::cerr << "  @" << addr << ": " << memory[addr] << " (0x" << std::hex << memory[addr]
                      << std::dec << ")";

            // Mark special addresses
            if (addr == ip)
                std::cerr << " <- IP";
            if (addr == sp)
                std::cerr << " <- SP";
            if (addr == bp)
                std::cerr << " <- BP";
            if (addr == hp)
                std::cerr << " <- HP";

            std::cerr << std::endl;
        }
    }

    // Disassemble bytecode starting at 'start' for 'count' instructions
    void disassemble(uint64_t start, uint64_t count) const {
        std::cerr << "=== Disassembly @ " << start << " ===" << std::endl;

        uint64_t addr = start;
        for (uint64_t i = 0; i < count && addr < MEMORY_SIZE; i++) {
            uint64_t instr = memory[addr];
            auto op = static_cast<Opcode>(instr & 0xFF);

            std::cerr << "  @" << addr << ": " << opcode_name(op);

            // Show immediate value for opcodes that use it
            switch (op) {
                case Opcode::PUSH:
                case Opcode::JMP:
                case Opcode::JZ:
                case Opcode::CALL:
                case Opcode::BP_LOAD:
                case Opcode::BP_STORE:
                    if (addr + 1 < MEMORY_SIZE) {
                        std::cerr << " " << memory[addr + 1];
                        addr++; // Skip immediate
                    }
                    break;
                default:
                    break;
            }

            if (addr == ip - 1)
                std::cerr << " <- IP";
            std::cerr << std::endl;
            addr++;
        }
    }
};

// Implement CompiledProgram::write_strings after StackVM is fully defined
inline void CompiledProgram::write_strings(StackVM& vm) const {
    for (const auto& str_lit : strings) {
        const std::string& str = str_lit.content;
        uint64_t addr = str_lit.address;

        // Write length
        vm.write_memory(addr, str.length());

        // Pack and write characters (8 per word)
        for (size_t word_idx = 0; word_idx < (str.length() + 7) / 8; word_idx++) {
            uint64_t word = 0;
            for (size_t byte_idx = 0; byte_idx < 8 && (word_idx * 8 + byte_idx) < str.length();
                 byte_idx++) {
                uint8_t ch = str[(word_idx * 8) + byte_idx];
                word |= (static_cast<uint64_t>(ch) << (byte_idx * 8));
            }
            vm.write_memory(addr + 1 + word_idx, word);
        }
    }
}
