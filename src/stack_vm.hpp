#pragma once
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <sys/mman.h>
#include <unistd.h>
#include <vector>

// Opcodes for the stack VM
enum class Opcode : uint8_t {
    HALT = 0,
    PUSH,  // Push immediate 64-bit value
    POP,   // Pop and discard
    DUP,   // Duplicate top
    SWAP,  // Swap top two
    ADD,   // Pop two, push sum
    SUB,   // Pop two, push difference
    MUL,   // Pop two, push product
    DIV,   // Pop two, push quotient
    MOD,   // Pop two, push remainder
    EQ,    // Pop two, push 1 if equal, 0 otherwise
    LT,    // Pop two, push 1 if less than, 0 otherwise
    GT,    // Pop two, push 1 if greater than, 0 otherwise
    JMP,   // Unconditional jump to address
    JZ,    // Jump if top of stack is zero
    ENTER, // Save previous BP to the stack set it to SP
    LEAVE, // Restore BP from the stack
    CALL,  // Call function at address
    RET,   // Return from function
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
};

class StackVM {
  private:
    uint64_t* memory; // Flat linear mmap'd memory (64-bit words)
    uint64_t ip{0};   // Instruction pointer
    uint64_t sp;      // Stack pointer
    uint64_t bp;      // Base/frame pointer
    uint64_t hp;      // Heap pointer
    bool running{false};

    // Memory layout (in 64-bit words):
    // [0 ... CODE_SIZE)           : Code segment
    // [CODE_SIZE ... STACK_BASE)  : Heap (grows upward from CODE_SIZE)
    // [STACK_BASE ... MEMORY_SIZE): Stack (grows downward from STACK_BASE)

    static constexpr size_t MEMORY_SIZE = 65536;      // 64K words (512KB total)
    static constexpr size_t CODE_SIZE = 16384;        // 16K words for code
    static constexpr size_t STACK_BASE = MEMORY_SIZE; // Stack grows down from end

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
    StackVM() : sp(STACK_BASE), bp(STACK_BASE), hp(CODE_SIZE) {
        // Allocate memory using mmap
        void* ptr = mmap(nullptr, MEMORY_SIZE * sizeof(uint64_t), PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        if (ptr == MAP_FAILED) {
            throw std::runtime_error("Failed to mmap memory");
        }

        memory = static_cast<uint64_t*>(ptr);
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

    // Execute the loaded program
    void execute() {
        ip = 0;
        sp = STACK_BASE;
        bp = STACK_BASE;
        running = true;

        while (running && ip < CODE_SIZE) {
            const uint64_t INSTRUCTION = memory[ip++];
            const auto OP = static_cast<Opcode>(INSTRUCTION & 0xFF);

            switch (OP) {
                case Opcode::HALT:
                    running = false;
                    break;

                case Opcode::PUSH:
                    if (ip >= CODE_SIZE)
                        throw std::runtime_error("IP out of bounds");
                    push(memory[ip++]);
                    break;

                case Opcode::POP:
                    pop();
                    break;

                case Opcode::DUP:
                    push(peek());
                    break;

                case Opcode::SWAP: {
                    uint64_t a = pop();
                    uint64_t b = pop();
                    push(a);
                    push(b);
                    break;
                }

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

                case Opcode::JMP:
                    if (ip >= CODE_SIZE)
                        throw std::runtime_error("IP out of bounds");
                    ip = memory[ip];
                    if (ip >= CODE_SIZE)
                        throw std::runtime_error("Jump target out of code segment");
                    break;

                case Opcode::JZ: {
                    if (ip >= CODE_SIZE)
                        throw std::runtime_error("IP out of bounds");
                    uint64_t cond = pop();
                    uint64_t addr = memory[ip++];
                    if (cond == 0) {
                        ip = addr;
                        if (ip >= CODE_SIZE)
                            throw std::runtime_error("Jump target out of code segment");
                    }
                    break;
                }

                case Opcode::ENTER: {
                    if (ip >= CODE_SIZE)
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
                    if (ip >= CODE_SIZE)
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
                    if (ip >= CODE_SIZE)
                        throw std::runtime_error("IP out of bounds");
                    const uint64_t TARGET = memory[ip++]; // Read target and advance IP
                    if (ip >= CODE_SIZE)
                        throw std::runtime_error("IP out of bounds");
                    const uint64_t NB_ARGS = memory[ip++]; // Arguments

                    const uint64_t BASE = sp;

                    push(ip);    // Save return address (now past the operand)
                    ip = TARGET; // Jump to function

                    for (uint64_t i = 0; i < NB_ARGS; i++) {
                        push(memory[BASE + NB_ARGS - 1 - i]);
                    }

                    if (ip >= CODE_SIZE)
                        throw std::runtime_error("Call target out of code segment");
                    break;
                }

                case Opcode::RET: {
                    if (ip >= CODE_SIZE)
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

                    if (ip >= CODE_SIZE && ip != CODE_SIZE) {
                        throw std::runtime_error("Return address out of code segment");
                    }

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
                        throw std::runtime_error("Cannot write to code segment");
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
                        throw std::runtime_error("Cannot write to code segment");
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

                default:
                    throw std::runtime_error("Unknown opcode");
            }
        }
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
