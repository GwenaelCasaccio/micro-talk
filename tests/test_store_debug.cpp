#include "stack_vm.hpp"
#include <iostream>

int main() {
    std::vector<uint64_t> bytecode;

    // PUSH 30000 (address)
    bytecode.push_back(static_cast<uint64_t>(Opcode::PUSH));
    bytecode.push_back(30000);

    // PUSH 42 (value)
    bytecode.push_back(static_cast<uint64_t>(Opcode::PUSH));
    bytecode.push_back(42);

    // STORE
    bytecode.push_back(static_cast<uint64_t>(Opcode::STORE));

    // HALT
    bytecode.push_back(static_cast<uint64_t>(Opcode::HALT));

    std::cout << "=== Direct STORE Test ===" << std::endl;
    std::cout << "Bytecode:" << std::endl;
    for (size_t i = 0; i < bytecode.size(); i++) {
        std::cout << i << ": " << bytecode[i] << std::endl;
    }

    try {
        StackVM vm;
        vm.load_program(bytecode);
        std::cout << "\nExecuting..." << std::endl;
        vm.execute();
        std::cout << "Success!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
