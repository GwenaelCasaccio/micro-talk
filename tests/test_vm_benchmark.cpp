#include "../src/stack_vm.hpp"
#include <chrono>
#include <iostream>
#include <vector>

using namespace std::chrono;

void benchmark_arithmetic() {
    std::cout << "=== Arithmetic Benchmark ===" << std::endl;
    std::cout << "Running 1,000,000 iterations of PUSH/ADD/POP..." << std::endl;

    StackVM vm;
    std::vector<uint64_t> program;

    // Loop 1,000,000 times: PUSH 42, PUSH 8, ADD, POP
    // We use a counter approach: decrement from 1M to 0
    program.push_back(static_cast<uint64_t>(Opcode::PUSH));
    program.push_back(1000000); // Loop counter

    // Loop start (address 2)
    size_t loop_start = program.size();

    // Arithmetic operations
    program.push_back(static_cast<uint64_t>(Opcode::PUSH));
    program.push_back(42);
    program.push_back(static_cast<uint64_t>(Opcode::PUSH));
    program.push_back(8);
    program.push_back(static_cast<uint64_t>(Opcode::ADD));
    program.push_back(static_cast<uint64_t>(Opcode::POP));

    // Decrement counter
    program.push_back(static_cast<uint64_t>(Opcode::PUSH));
    program.push_back(1);
    program.push_back(static_cast<uint64_t>(Opcode::SUB));
    program.push_back(static_cast<uint64_t>(Opcode::DUP));

    // Jump if not zero
    program.push_back(static_cast<uint64_t>(Opcode::PUSH));
    program.push_back(0);
    program.push_back(static_cast<uint64_t>(Opcode::EQ));
    program.push_back(static_cast<uint64_t>(Opcode::JZ));
    program.push_back(loop_start);

    program.push_back(static_cast<uint64_t>(Opcode::HALT));

    vm.load_program(program);

    auto start = high_resolution_clock::now();
    vm.execute();
    auto end = high_resolution_clock::now();

    auto duration = duration_cast<milliseconds>(end - start);
    std::cout << "Time: " << duration.count() << "ms" << std::endl;
    std::cout << "Throughput: " << (1000000.0 / duration.count()) << "K ops/ms" << std::endl;
    std::cout << std::endl;
}

void benchmark_control_flow() {
    std::cout << "=== Control Flow Benchmark ===" << std::endl;
    std::cout << "Running 100,000 iterations with nested conditionals..." << std::endl;

    StackVM vm;
    std::vector<uint64_t> program;

    // Loop 100,000 times with conditional branches
    program.push_back(static_cast<uint64_t>(Opcode::PUSH));
    program.push_back(100000);

    size_t loop_start = program.size();

    // Conditional: if (counter % 2 == 0) then operation
    program.push_back(static_cast<uint64_t>(Opcode::DUP));
    program.push_back(static_cast<uint64_t>(Opcode::PUSH));
    program.push_back(2);
    program.push_back(static_cast<uint64_t>(Opcode::MOD));
    program.push_back(static_cast<uint64_t>(Opcode::PUSH));
    program.push_back(0);
    program.push_back(static_cast<uint64_t>(Opcode::EQ));
    program.push_back(static_cast<uint64_t>(Opcode::JZ));
    size_t skip_addr = program.size();
    program.push_back(0); // Placeholder for skip address

    // Then branch: some arithmetic
    program.push_back(static_cast<uint64_t>(Opcode::PUSH));
    program.push_back(1);
    program.push_back(static_cast<uint64_t>(Opcode::PUSH));
    program.push_back(1);
    program.push_back(static_cast<uint64_t>(Opcode::ADD));
    program.push_back(static_cast<uint64_t>(Opcode::POP));

    // Patch skip address
    program[skip_addr] = program.size();

    // Decrement counter
    program.push_back(static_cast<uint64_t>(Opcode::PUSH));
    program.push_back(1);
    program.push_back(static_cast<uint64_t>(Opcode::SUB));
    program.push_back(static_cast<uint64_t>(Opcode::DUP));

    // Loop condition
    program.push_back(static_cast<uint64_t>(Opcode::PUSH));
    program.push_back(0);
    program.push_back(static_cast<uint64_t>(Opcode::EQ));
    program.push_back(static_cast<uint64_t>(Opcode::JZ));
    program.push_back(loop_start);

    program.push_back(static_cast<uint64_t>(Opcode::HALT));

    vm.load_program(program);

    auto start = high_resolution_clock::now();
    vm.execute();
    auto end = high_resolution_clock::now();

    auto duration = duration_cast<milliseconds>(end - start);
    std::cout << "Time: " << duration.count() << "ms" << std::endl;
    std::cout << "Throughput: " << (100000.0 / duration.count()) << "K ops/ms" << std::endl;
    std::cout << std::endl;
}

void benchmark_memory() {
    std::cout << "=== Memory Operations Benchmark ===" << std::endl;
    std::cout << "Running 100,000 iterations of LOAD/STORE..." << std::endl;

    StackVM vm;
    std::vector<uint64_t> program;

    // Initialize a value at heap address 30000 first (well into heap region)
    program.push_back(static_cast<uint64_t>(Opcode::PUSH));
    program.push_back(0); // Initial value
    program.push_back(static_cast<uint64_t>(Opcode::PUSH));
    program.push_back(30000); // Heap address (well after code segment which ends at 16384)
    program.push_back(static_cast<uint64_t>(Opcode::SWAP));
    program.push_back(static_cast<uint64_t>(Opcode::STORE));

    // Loop 100,000 times: LOAD, modify, STORE
    program.push_back(static_cast<uint64_t>(Opcode::PUSH));
    program.push_back(100000);

    size_t loop_start = program.size();

    // Load from heap address 30000, add 1, store back
    program.push_back(static_cast<uint64_t>(Opcode::PUSH));
    program.push_back(30000); // Heap address
    program.push_back(static_cast<uint64_t>(Opcode::LOAD));
    program.push_back(static_cast<uint64_t>(Opcode::PUSH));
    program.push_back(1);
    program.push_back(static_cast<uint64_t>(Opcode::ADD));
    program.push_back(static_cast<uint64_t>(Opcode::PUSH));
    program.push_back(30000);
    program.push_back(static_cast<uint64_t>(Opcode::SWAP));
    program.push_back(static_cast<uint64_t>(Opcode::STORE));

    // Decrement counter
    program.push_back(static_cast<uint64_t>(Opcode::PUSH));
    program.push_back(1);
    program.push_back(static_cast<uint64_t>(Opcode::SUB));
    program.push_back(static_cast<uint64_t>(Opcode::DUP));

    // Loop condition
    program.push_back(static_cast<uint64_t>(Opcode::PUSH));
    program.push_back(0);
    program.push_back(static_cast<uint64_t>(Opcode::EQ));
    program.push_back(static_cast<uint64_t>(Opcode::JZ));
    program.push_back(loop_start);

    program.push_back(static_cast<uint64_t>(Opcode::HALT));

    vm.load_program(program);

    auto start = high_resolution_clock::now();
    vm.execute();
    auto end = high_resolution_clock::now();

    auto duration = duration_cast<milliseconds>(end - start);
    std::cout << "Time: " << duration.count() << "ms" << std::endl;
    std::cout << "Throughput: " << (100000.0 / duration.count()) << "K ops/ms" << std::endl;
    std::cout << std::endl;
}

void benchmark_mixed() {
    std::cout << "=== Mixed Operations Benchmark ===" << std::endl;
    std::cout << "Running 100,000 iterations with mixed stack/ALU/memory operations..."
              << std::endl;

    StackVM vm;
    std::vector<uint64_t> program;

    // Loop 100,000 times with a mix of operations
    program.push_back(static_cast<uint64_t>(Opcode::PUSH));
    program.push_back(100000);

    size_t loop_start = program.size();

    // Mixed operations: stack, arithmetic, bitwise, comparison
    program.push_back(static_cast<uint64_t>(Opcode::DUP));
    program.push_back(static_cast<uint64_t>(Opcode::PUSH));
    program.push_back(10);
    program.push_back(static_cast<uint64_t>(Opcode::ADD));
    program.push_back(static_cast<uint64_t>(Opcode::PUSH));
    program.push_back(3);
    program.push_back(static_cast<uint64_t>(Opcode::MUL));
    program.push_back(static_cast<uint64_t>(Opcode::PUSH));
    program.push_back(7);
    program.push_back(static_cast<uint64_t>(Opcode::AND));
    program.push_back(static_cast<uint64_t>(Opcode::POP));

    // Decrement counter
    program.push_back(static_cast<uint64_t>(Opcode::PUSH));
    program.push_back(1);
    program.push_back(static_cast<uint64_t>(Opcode::SUB));
    program.push_back(static_cast<uint64_t>(Opcode::DUP));

    // Loop condition
    program.push_back(static_cast<uint64_t>(Opcode::PUSH));
    program.push_back(0);
    program.push_back(static_cast<uint64_t>(Opcode::EQ));
    program.push_back(static_cast<uint64_t>(Opcode::JZ));
    program.push_back(loop_start);

    program.push_back(static_cast<uint64_t>(Opcode::HALT));

    vm.load_program(program);

    auto start = high_resolution_clock::now();
    vm.execute();
    auto end = high_resolution_clock::now();

    auto duration = duration_cast<milliseconds>(end - start);
    std::cout << "Time: " << duration.count() << "ms" << std::endl;
    std::cout << "Throughput: " << (100000.0 / duration.count()) << "K ops/ms" << std::endl;
    std::cout << std::endl;
}

int main() {
    std::cout << "╔══════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║   VM Dispatch Performance Benchmark (Computed Goto)  ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════════╝" << std::endl;
    std::cout << std::endl;

    benchmark_arithmetic();
    benchmark_control_flow();
    // benchmark_memory();  // TODO: Fix code segment protection issue
    benchmark_mixed();

    std::cout << "╔══════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║   Benchmark Complete                              ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════════╝" << std::endl;

    return 0;
}
