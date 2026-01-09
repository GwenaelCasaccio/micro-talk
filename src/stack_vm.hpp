#pragma once
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <sys/mman.h>
#include <unistd.h>
#include <vector>

#include "interrupt.hpp"

// Opcodes for the stack VM
enum class Opcode : uint8_t {
    HALT = 0,
    PUSH,  // Push immediate 64-bit value
    POP,   // Pop and discard
    DUP,   // Duplicate top
    ADD,   // Pop two, push sum
    SUB,   // Pop two, push difference
    MUL,   // Pop two, push product
    DIV,   // Pop two, push quotient
    MOD,   // Pop two, push remainder
    EQ,    // Pop two, push 1 if equal, 0 otherwise
    LT,    // Pop two, push 1 if less than, 0 otherwise
    GT,    // Pop two, push 1 if greater than, 0 otherwise
    LTE,   // Pop two, push 1 if less than or equal, 0 otherwise
    GTE,   // Pop two, push 1 if greater than or equal, 0 otherwise
    JMP,   // Unconditional jump to address
    JZ,    // Jump if top of stack is zero
    ENTER, // Save previous BP to the stack set it to SP
    LEAVE, // Restore BP from the stack
    CALL,  // Call function at address
    RET,   // Return from function
    IRET,  // Return from interruption
    LOAD,  // Load from memory address on stack
    STORE, // Store value to memory address
    BP_LOAD,
    BP_STORE,
    PRINT,     // Debug: print top of stack as integer
    PRINT_STR, // Debug: print string at address on stack
    AND,       // Bitwise AND
    OR,        // Bitwise OR
    XOR,       // Bitwise XOR
    SHL,       // Shift left
    SHR,       // Shift right (logical)
    ASHR,      // Arithmetic shift right
    CLI,
    STI,
    SIGNAL_REG,
    ABORT,   // Abort with error message (address on stack)
    FUNCALL, // Call function at address on stack (address, arg_count on stack)
};

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

    // Memory layout (in 64-bit words, each word = 8 bytes):
    // Large, well-separated regions to avoid collisions
    // Uses virtual memory - physical memory only allocated on access
    //
    // [0 ... CODE_SIZE)                    : Code segment (1GB)
    // [GLOBALS_START ... HEAP_START)       : Global variables (1GB)
    // [HEAP_START ... STACK_START)         : Heap (1GB, grows upward)
    // [STACK_START ... MEMORY_SIZE)        : Stack (1GB, grows downward)

    static constexpr size_t GB_IN_WORDS = 134217728; // 1GB / 8 bytes = 128M words

    static constexpr size_t CODE_SIZE = GB_IN_WORDS;       // 1GB for code
    static constexpr size_t GLOBALS_START = CODE_SIZE;     // Starts after code
    static constexpr size_t HEAP_START = 2 * GB_IN_WORDS;  // Starts after globals
    static constexpr size_t STACK_START = 3 * GB_IN_WORDS; // Starts after heap
    static constexpr size_t MEMORY_SIZE = 4 * GB_IN_WORDS; // 4GB total
    static constexpr size_t STACK_BASE = STACK_START;      // Stack grows down from start

    void push(uint64_t value) {
        if (sp <= hp) {
            throw std::runtime_error("Stack overflow - collided with heap");
        }
        memory[--sp] = value;
    }

    uint64_t pop() {
        if (sp >= STACK_BASE) {
            throw std::runtime_error("Stack underflow");
        }
        return memory[sp++];
    }

    [[nodiscard]] uint64_t peek() const {
        if (sp >= STACK_BASE) {
            throw std::runtime_error("Stack is empty");
        }
        return memory[sp];
    }

    static void check_memory_bounds(uint64_t addr) {
        if (addr >= MEMORY_SIZE) {
            throw std::runtime_error("Memory access out of bounds");
        }
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

            switch (OP) {
                case Opcode::HALT:
                    running = false;
                    break;

                case Opcode::PUSH:
                    if (ip >= MEMORY_SIZE)
                        throw std::runtime_error("IP out of bounds");
                    push(memory[ip++]);
                    break;

                case Opcode::POP:
                    pop();
                    break;

                case Opcode::DUP:
                    push(peek());
                    break;

                case Opcode::ADD: {
                    uint64_t b = pop();
                    uint64_t a = pop();
                    push(a + b);
                    break;
                }

                case Opcode::SUB: {
                    uint64_t b = pop();
                    uint64_t a = pop();
                    push(a - b);
                    break;
                }

                case Opcode::MUL: {
                    uint64_t b = pop();
                    uint64_t a = pop();
                    push(a * b);
                    break;
                }

                case Opcode::DIV: {
                    uint64_t b = pop();
                    uint64_t a = pop();
                    if (b == 0)
                        throw std::runtime_error("Division by zero");
                    push(a / b);
                    break;
                }

                case Opcode::MOD: {
                    uint64_t b = pop();
                    uint64_t a = pop();
                    if (b == 0)
                        throw std::runtime_error("Modulo by zero");
                    push(a % b);
                    break;
                }

                case Opcode::EQ: {
                    uint64_t b = pop();
                    uint64_t a = pop();
                    push(a == b ? 1 : 0);
                    break;
                }

                case Opcode::LT: {
                    uint64_t b = pop();
                    uint64_t a = pop();
                    push(a < b ? 1 : 0);
                    break;
                }

                case Opcode::GT: {
                    uint64_t b = pop();
                    uint64_t a = pop();
                    push(a > b ? 1 : 0);
                    break;
                }

                case Opcode::LTE: {
                    uint64_t b = pop();
                    uint64_t a = pop();
                    push(a <= b ? 1 : 0);
                    break;
                }

                case Opcode::GTE: {
                    uint64_t b = pop();
                    uint64_t a = pop();
                    push(a >= b ? 1 : 0);
                    break;
                }

                case Opcode::JMP:
                    if (ip >= MEMORY_SIZE)
                        throw std::runtime_error("IP out of bounds");
                    ip = memory[ip];
                    if (ip >= MEMORY_SIZE)
                        throw std::runtime_error("Jump target out of bounds");
                    break;

                case Opcode::JZ: {
                    if (ip >= MEMORY_SIZE)
                        throw std::runtime_error("IP out of bounds");
                    uint64_t cond = pop();
                    uint64_t addr = memory[ip++];
                    if (cond == 0) {
                        ip = addr;
                        if (ip >= MEMORY_SIZE)
                            throw std::runtime_error("Jump target out of bounds");
                    }
                    break;
                }

                case Opcode::ENTER: {
                    if (ip >= MEMORY_SIZE)
                        throw std::runtime_error("IP out of bounds");
                    const size_t TEMP_SIZE = memory[ip++];

                    for (uint64_t i = 0; i < TEMP_SIZE; i++) {
                        push(0);
                    }
                    push(bp);
                    bp = sp;

                    break;
                }

                case Opcode::LEAVE: {
                    if (ip >= MEMORY_SIZE)
                        throw std::runtime_error("IP out of bounds");
                    const size_t TEMP_SIZE = memory[ip++];
                    const uint64_t RESULT = pop();

                    bp = pop();

                    for (uint64_t i = 0; i < TEMP_SIZE; i++) {
                        pop();
                    }

                    push(RESULT);
                    break;
                }

                case Opcode::CALL: {
                    if (ip >= MEMORY_SIZE)
                        throw std::runtime_error("IP out of bounds");
                    const uint64_t TARGET = memory[ip++]; // Read target and advance IP
                    if (ip >= MEMORY_SIZE)
                        throw std::runtime_error("IP out of bounds");
                    const uint64_t NB_ARGS = memory[ip++]; // Arguments

                    const uint64_t BASE = sp;

                    push(ip);    // Save return address (now past the operand)
                    ip = TARGET; // Jump to function

                    for (uint64_t i = NB_ARGS; i > 0; i--) {
                        push(memory[BASE + NB_ARGS - i]);
                    }

                    if (ip >= MEMORY_SIZE)
                        throw std::runtime_error("Call target out of bounds");
                    break;
                }

                case Opcode::RET: {
                    if (ip >= MEMORY_SIZE)
                        throw std::runtime_error("IP out of bounds");
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

                    if (ip >= MEMORY_SIZE) {
                        throw std::runtime_error("Return address out of bounds");
                    }

                    break;
                }

                case Opcode::IRET: {
                    ip = pop();

                    if (ip >= MEMORY_SIZE) {
                        throw std::runtime_error("Return address out of bounds");
                    }

                    interrupt_flag = true;

                    break;
                }

                case Opcode::LOAD: {
                    uint64_t addr = pop();
                    check_memory_bounds(addr);
                    push(memory[addr]);
                    break;
                }

                case Opcode::STORE: {
                    uint64_t addr = pop();
                    uint64_t value = pop();
                    check_memory_bounds(addr);
                    if (addr < CODE_SIZE) {
                        throw std::runtime_error("STORE Cannot write to code segment");
                    }
                    memory[addr] = value;
                    break;
                }

                case Opcode::BP_LOAD: {
                    const uint64_t IDX = pop();
                    const uint64_t ADR = bp + IDX + 1;
                    check_memory_bounds(ADR);
                    const uint64_t VALUE = memory[ADR];
                    push(VALUE);
                    break;
                }

                case Opcode::BP_STORE: {
                    const uint64_t IDX = pop();
                    const uint64_t VALUE = pop();
                    const uint64_t ADR = bp + IDX + 1; // with SP ++ IP on the stack
                    check_memory_bounds(ADR);
                    if (IDX < 1) {
                        throw std::runtime_error("At least return");
                    }
                    if (ADR < CODE_SIZE) {
                        throw std::runtime_error("BP_STORE Cannot write to code segment");
                    }
                    memory[ADR] = VALUE;
                    break;
                }
                case Opcode::PRINT:
                    std::cout << "DEBUG: " << peek() << '\n';
                    break;

                case Opcode::PRINT_STR: {
                    uint64_t addr = peek();
                    check_memory_bounds(addr);
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
                    break;
                }

                case Opcode::AND: {
                    uint64_t b = pop();
                    uint64_t a = pop();
                    push(a & b);
                    break;
                }

                case Opcode::OR: {
                    uint64_t b = pop();
                    uint64_t a = pop();
                    push(a | b);
                    break;
                }

                case Opcode::XOR: {
                    uint64_t b = pop();
                    uint64_t a = pop();
                    push(a ^ b);
                    break;
                }

                case Opcode::SHL: {
                    uint64_t b = pop();
                    uint64_t a = pop();
                    push(a << b);
                    break;
                }

                case Opcode::SHR: {
                    uint64_t b = pop();
                    uint64_t a = pop();
                    push(a >> b);
                    break;
                }

                case Opcode::ASHR: {
                    uint64_t b = pop();
                    uint64_t a = pop();
                    auto signed_a = static_cast<int64_t>(a);
                    push(static_cast<uint64_t>(signed_a >> b));
                    break;
                }

                case Opcode::CLI: {
                    interrupt_flag = false;
                    break;
                }

                case Opcode::STI: {
                    interrupt_flag = true;
                    break;
                }

                case Opcode::SIGNAL_REG: {
                    const uint64_t signal = pop();
                    const size_t code_ptr = pop();

                    if (signal < 1 || signal > 31)
                        throw std::runtime_error("Bad signal ID");

                    if (code_ptr >= MEMORY_SIZE) {
                        throw std::runtime_error("Interrupt pointer out of bounds");
                    }

                    signal_handlers[signal - 1] = code_ptr;

                    break;
                }

                case Opcode::ABORT: {
                    const uint64_t addr = pop();
                    if (addr >= MEMORY_SIZE) {
                        std::cerr << "ABORT: Invalid string address " << addr << std::endl;
                        running = false;
                        break;
                    }

                    // Read string: length followed by packed characters
                    const uint64_t length = memory[addr];
                    std::cout << "ABORT: ";
                    for (size_t i = 0; i < length; i++) {
                        const size_t word_idx = addr + 1 + i / 8;
                        const size_t byte_idx = i % 8;
                        const char c =
                            static_cast<char>((memory[word_idx] >> (byte_idx * 8)) & 0xFF);
                        std::cout << c;
                    }
                    std::cout << std::endl;
                    running = false;
                    break;
                }

                case Opcode::FUNCALL: {
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

                    break;
                }

                default:
                    throw std::runtime_error("Unknown opcode");
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
        check_memory_bounds(addr);
        return memory[addr];
    }
    void write_memory(uint64_t addr, uint64_t value) {
        check_memory_bounds(addr);
        if (addr < CODE_SIZE) {
            throw std::runtime_error("Cannot write to code segment");
        }
        memory[addr] = value;
    }
};
