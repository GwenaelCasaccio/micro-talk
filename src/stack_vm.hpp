#pragma once
#include <cstdint>
#include <vector>
#include <stdexcept>
#include <cstring>
#include <sys/mman.h>
#include <unistd.h>
#include <iostream>

// Opcodes for the stack VM
enum class Opcode : uint8_t {
    HALT = 0,
    PUSH,      // Push immediate 64-bit value
    POP,       // Pop and discard
    DUP,       // Duplicate top
    SWAP,      // Swap top two
    ADD,       // Pop two, push sum
    SUB,       // Pop two, push difference
    MUL,       // Pop two, push product
    DIV,       // Pop two, push quotient
    MOD,       // Pop two, push remainder
    EQ,        // Pop two, push 1 if equal, 0 otherwise
    LT,        // Pop two, push 1 if less than, 0 otherwise
    GT,        // Pop two, push 1 if greater than, 0 otherwise
    JMP,       // Unconditional jump to address
    JZ,        // Jump if top of stack is zero
    ENTER,     // Save previous BP to the stack set it to SP
    LEAVE,     // Restore BP from the stack
    CALL,      // Call function at address
    RET,       // Return from function
    LOAD,      // Load from memory address on stack
    STORE,     // Store value to memory address
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
    uint64_t* memory;                  // Flat linear mmap'd memory (64-bit words)
    uint64_t ip;                        // Instruction pointer
    uint64_t sp;                        // Stack pointer
    uint64_t bp;                        // Base/frame pointer
    uint64_t hp;                        // Heap pointer
    bool running;
    
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
    
    uint64_t peek() const {
        if (sp >= STACK_BASE) {
            throw std::runtime_error("Stack is empty");
        }
        return memory[sp];
    }
    
    void check_memory_bounds(uint64_t addr) const {
        if (addr >= MEMORY_SIZE) {
            throw std::runtime_error("Memory access out of bounds");
        }
    }
    
public:
    StackVM() : ip(0), sp(STACK_BASE), bp(STACK_BASE), hp(CODE_SIZE), running(false) {
        // Allocate memory using mmap
        void* ptr = mmap(nullptr, 
                        MEMORY_SIZE * sizeof(uint64_t),
                        PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS,
                        -1, 
                        0);
        
        if (ptr == MAP_FAILED) {
            throw std::runtime_error("Failed to mmap memory");
        }
        
        memory = static_cast<uint64_t*>(ptr);
    }
    
    ~StackVM() {
        if (memory) {
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
            const uint64_t instruction = memory[ip++];
            const Opcode op = static_cast<Opcode>(instruction & 0xFF);
            
            switch (op) {
                case Opcode::HALT:
                    running = false;
                    break;
                    
                case Opcode::PUSH:
                    if (ip >= CODE_SIZE) throw std::runtime_error("IP out of bounds");
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
                    if (b == 0) throw std::runtime_error("Division by zero");
                    push(a / b);
                    break;
                }
                
                case Opcode::MOD: {
                    uint64_t b = pop();
                    uint64_t a = pop();
                    if (b == 0) throw std::runtime_error("Modulo by zero");
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
                    if (ip >= CODE_SIZE) throw std::runtime_error("IP out of bounds");
                    ip = memory[ip];
                    if (ip >= CODE_SIZE) throw std::runtime_error("Jump target out of code segment");
                    break;
                    
                case Opcode::JZ: {
                    if (ip >= CODE_SIZE) throw std::runtime_error("IP out of bounds");
                    uint64_t cond = pop();
                    uint64_t addr = memory[ip++];
                    if (cond == 0) {
                        ip = addr;
                        if (ip >= CODE_SIZE) throw std::runtime_error("Jump target out of code segment");
                    }
                    break;
                }
               
		case Opcode::ENTER: {
		    push(bp);
		    bp = sp;
		    break;
		} 

		case Opcode::LEAVE: {
		   bp = pop();
		    break;
		}

                case Opcode::CALL: {
                    if (ip >= CODE_SIZE) throw std::runtime_error("IP out of bounds");
                    uint64_t target = memory[ip++];  // Read target and advance IP
                    push(ip);      // Save return address (now past the operand)
                    ip = target;   // Jump to function
                    if (ip >= CODE_SIZE) throw std::runtime_error("Call target out of code segment");
                    break;
                }
                    
                case Opcode::RET: {
                    // Simple return: pop result, pop return address, push result, jump
                    uint64_t result = pop();      // Pop return value
                    uint64_t ret_addr = pop();    // Pop return address
                    push(result);                  // Push result back
                    ip = ret_addr;                 // Return to caller
                    
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
                
                case Opcode::PRINT:
	            std::cout << "DEBUG: " << peek() << std::endl;
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
		    std::cout << std::endl;
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
                    int64_t signed_a = static_cast<int64_t>(a);
                    push(static_cast<uint64_t>(signed_a >> b));
                    break;
                }
                    
                default:
                    throw std::runtime_error("Unknown opcode");
            }
        }
    }
    
    uint64_t get_top() const {
        return peek();
    }
    
    // Debug accessors
    uint64_t get_sp() const { return sp; }
    uint64_t get_bp() const { return bp; }
    
    // Direct memory access (for debugging/inspection)
    uint64_t read_memory(uint64_t addr) const {
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
