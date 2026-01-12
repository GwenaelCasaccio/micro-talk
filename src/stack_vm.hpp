#pragma once
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <sys/mman.h>
#include <unistd.h>
#include <vector>

#include "compiled_program.hpp"
#include "interrupt.hpp"
#include "memory_layout.hpp"
#include "opcodes.hpp"
#include "vm_checks.hpp"

// ============================================================================
// Stack Virtual Machine
// ============================================================================
// A 64-bit stack-based virtual machine with flat memory model and
// computed goto dispatch for high-performance bytecode execution.

class StackVM {
  private:
    InterruptHandling interruptHandling;
    bool interrupt_flag{true};
    std::array<uint64_t, 31> signal_handlers{0};

    uint64_t* memory; // Flat linear mmap'd memory (64-bit words)
    uint64_t ip{0};   // Instruction pointer
    uint64_t sp;      // Stack pointer
    uint64_t bp;      // Base/frame pointer
    uint64_t hp;      // Heap pointer
    bool running{false};
    bool trace_mode{false}; // Enable trace/debug output during execution

    // Memory layout constants are defined in memory_layout.hpp
    // See MemoryLayout namespace for region boundaries and helper functions
    static constexpr size_t CODE_SIZE = MemoryLayout::CODE_SIZE;
    static constexpr size_t GLOBALS_START = MemoryLayout::GLOBALS_START;
    static constexpr size_t HEAP_START = MemoryLayout::HEAP_START;
    static constexpr size_t STACK_START = MemoryLayout::STACK_START;
    static constexpr size_t MEMORY_SIZE = MemoryLayout::MEMORY_SIZE;
    static constexpr size_t STACK_BASE = MemoryLayout::STACK_BASE;

    inline void push(uint64_t value) {
        VMChecks::check_stack_overflow(sp, hp);
        memory[--sp] = value;
    }

    inline uint64_t pop() {
        VMChecks::check_stack_underflow(sp);
        return memory[sp++];
    }

    [[nodiscard]] inline uint64_t peek() const {
        VMChecks::check_stack_empty(sp);
        return memory[sp];
    }

  public:
    StackVM() : sp(STACK_BASE), bp(STACK_BASE), hp(HEAP_START) {
        // Allocate memory using mmap
        void* ptr = mmap(nullptr, MEMORY_SIZE * sizeof(uint64_t), PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        if (ptr == MAP_FAILED) {
            throw std::runtime_error("Failed to mmap memory");
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
            throw std::runtime_error("Program too large for code segment");
        }
        memcpy(memory, program.data(), program.size() * sizeof(uint64_t));
    }

    // Load compiled program (bytecode + data)
    void load_program(const CompiledProgram& program) {
        // Load bytecode
        if (program.bytecode.size() > CODE_SIZE) {
            throw std::runtime_error("Program too large for code segment");
        }
        memcpy(memory, program.bytecode.data(), program.bytecode.size() * sizeof(uint64_t));

        // Write string literals to memory
        program.write_strings(*this);
    }

    void reset() {
        ip = 0;
        sp = STACK_BASE;
        bp = STACK_BASE;
        running = true;
        interruptHandling.clear();
    }

    // Execute the loaded program
    void execute() {
        while (running && ip < MEMORY_SIZE) {
            if (interrupt_flag && interruptHandling.has_event()) {
                for (int sig = 1; sig <= 31; sig++) {
                    if (interruptHandling.get_count(sig) > 0 && signal_handlers[sig - 1] != 0) {
                        interrupt_flag = false;
                        push(ip);
                        ip = signal_handlers[sig - 1];
                        interruptHandling.consumme(sig);
                        break;
                    }
                }
            }

            const uint64_t INSTRUCTION = memory[ip++];
            const auto OP = static_cast<Opcode>(INSTRUCTION & 0xFF);

            if (trace_mode) {
                std::cerr << "IP=" << (ip - 1) << " OP=" << opcode_name(OP) << " SP=" << sp
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
                &&op_signal_reg, &&op_abort,     &&op_funcall};

            const uint8_t opcode_idx = static_cast<uint8_t>(OP);
            if (opcode_idx >= 43) {
                throw std::runtime_error("Unknown opcode");
            }
            goto* dispatch_table[opcode_idx];

        op_halt:
            running = false;
            continue;

        op_push:
            VMChecks::check_ip_bounds(ip);
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
                throw std::runtime_error("Division by zero");
            push(a / b);
            continue;
        }

        op_mod: {
            uint64_t b = pop();
            uint64_t a = pop();
            if (b == 0)
                throw std::runtime_error("Modulo by zero");
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
            VMChecks::check_ip_bounds(ip);
            ip = memory[ip];
            VMChecks::check_ip_bounds(ip);
            continue;

        op_jz: {
            VMChecks::check_ip_bounds(ip);
            uint64_t cond = pop();
            uint64_t addr = memory[ip++];
            if (cond == 0) {
                ip = addr;
                VMChecks::check_ip_bounds(ip);
            }
            continue;
        }

        op_enter: {
            VMChecks::check_ip_bounds(ip);
            const size_t TEMP_SIZE = memory[ip++];

            for (uint64_t i = 0; i < TEMP_SIZE; i++) {
                push(0);
            }
            push(bp);
            bp = sp;

            continue;
        }

        op_leave: {
            VMChecks::check_ip_bounds(ip);
            const size_t TEMP_SIZE = memory[ip++];
            const uint64_t RESULT = pop();

            bp = pop();

            for (uint64_t i = 0; i < TEMP_SIZE; i++) {
                pop();
            }

            push(RESULT);
            continue;
        }

        op_call: {
            VMChecks::check_ip_bounds(ip);
            const uint64_t TARGET = memory[ip++]; // Read target and advance IP
            VMChecks::check_ip_bounds(ip);
            const uint64_t NB_ARGS = memory[ip++]; // Arguments

            const uint64_t BASE = sp;

            push(ip);    // Save return address (now past the operand)
            ip = TARGET; // Jump to function

            for (uint64_t i = NB_ARGS; i > 0; i--) {
                push(memory[BASE + NB_ARGS - i]);
            }

            VMChecks::check_ip_bounds(ip);
            continue;
        }

        op_ret: {
            VMChecks::check_ip_bounds(ip);
            const size_t NB_ARGS = memory[ip++];

            const uint64_t RESULT = pop();

            for (uint64_t i = 0; i < NB_ARGS; i++) {
                pop();
            }

            const uint64_t RET_ADDR = pop(); // Pop return address

            for (uint64_t i = 0; i < NB_ARGS; i++) {
                pop();
            }

            push(RESULT);

            ip = RET_ADDR; // Return to caller

            VMChecks::check_ip_bounds(ip);

            continue;
        }

        op_iret: {
            ip = pop();

            VMChecks::check_ip_bounds(ip);

            interrupt_flag = true;

            continue;
        }

        op_load: {
            uint64_t addr = pop();
            VMChecks::check_memory_bounds(addr);
            push(memory[addr]);
            continue;
        }

        op_store: {
            uint64_t addr = pop();
            uint64_t value = pop();
            VMChecks::check_memory_bounds(addr);
            VMChecks::check_code_segment_protection(addr);
            memory[addr] = value;
            continue;
        }

        op_load_byte: {
            // Load a single byte from memory
            // Stack: [addr] -> [byte_value]
            uint64_t addr = pop();
            VMChecks::check_memory_bounds(addr / 8); // Check word boundary

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

            VMChecks::check_memory_bounds(addr / 8);
            uint64_t word_idx = addr / 8;

            VMChecks::check_code_segment_protection(word_idx);

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
            VMChecks::check_memory_bounds(addr / 8);

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

            VMChecks::check_memory_bounds(addr / 8);
            uint64_t word_idx = addr / 8;

            VMChecks::check_code_segment_protection(word_idx);

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
            const uint64_t IDX = pop();
            const uint64_t ADR = bp + IDX + 1;
            VMChecks::check_memory_bounds(ADR);
            VMChecks::check_stack_frame_bounds(ADR, sp);
            const uint64_t VALUE = memory[ADR];
            push(VALUE);
            continue;
        }

        op_bp_store: {
            const uint64_t IDX = pop();
            const uint64_t VALUE = pop();
            const uint64_t ADR = bp + IDX + 1; // with SP ++ IP on the stack
            VMChecks::check_memory_bounds(ADR);
            VMChecks::check_bp_store_index(IDX);
            VMChecks::check_stack_frame_bounds(ADR, sp);
            VMChecks::check_code_segment_protection(ADR);
            memory[ADR] = VALUE;
            continue;
        }

        op_print:
            std::cout << "DEBUG: " << peek() << '\n';
            continue;

        op_print_str: {
            uint64_t addr = peek();
            VMChecks::check_memory_bounds(addr);
            // Read length from first word
            uint64_t len = memory[addr];
            std::cout << "DEBUG_STR: ";
            // Read characters (packed as 8 bytes per word)
            for (uint64_t i = 0; i < len; i++) {
                uint64_t word_idx = (i / 8) + 1;
                uint64_t byte_idx = i % 8;
                uint64_t word = memory[addr + word_idx];
                char c = (word >> (byte_idx * 8)) & 0xFF;
                std::cout << c;
            }
            std::cout << '\n';
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

            if (signal < 1 || signal > 31)
                throw std::runtime_error("Bad signal ID");

            if (code_ptr >= MEMORY_SIZE) {
                throw std::runtime_error("Interrupt pointer out of bounds");
            }

            signal_handlers[signal - 1] = code_ptr;

            continue;
        }

        op_abort: {
            const uint64_t addr = pop();
            if (addr >= MEMORY_SIZE) {
                std::cerr << "ABORT: Invalid string address " << addr << std::endl;
                running = false;
                continue;
            }

            // Read string: length followed by packed characters
            const uint64_t length = memory[addr];
            std::cout << "ABORT: ";
            for (size_t i = 0; i < length; i++) {
                const size_t word_idx = addr + 1 + i / 8;
                const size_t byte_idx = i % 8;
                const char c = static_cast<char>((memory[word_idx] >> (byte_idx * 8)) & 0xFF);
                std::cout << c;
            }
            std::cout << std::endl;
            running = false;
            continue;
        }

        op_funcall: {
            // Stack: [arg1] [arg2] ... [argN] [arg_count] [target_address]
            const uint64_t TARGET = pop();
            const uint64_t NB_ARGS = pop();

            if (TARGET >= MEMORY_SIZE)
                throw std::runtime_error("FUNCALL target out of bounds");

            const uint64_t BASE = sp;

            push(ip);    // Save return address
            ip = TARGET; // Jump to function

            // Push arguments to new frame
            for (uint64_t i = NB_ARGS; i > 0; i--) {
                push(memory[BASE + NB_ARGS - i]);
            }

            continue;
        }
        }

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
        if (signal < 1 || signal > 31)
            throw std::runtime_error("Invalid signal");
        return signal_handlers[signal - 1];
    }

    [[nodiscard]] uint64_t read_memory(uint64_t addr) const {
        VMChecks::check_memory_bounds(addr);
        return memory[addr];
    }
    void write_memory(uint64_t addr, uint64_t value) {
        VMChecks::check_memory_bounds(addr);
        if (addr < CODE_SIZE) {
            throw std::runtime_error("Cannot write to code segment");
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
