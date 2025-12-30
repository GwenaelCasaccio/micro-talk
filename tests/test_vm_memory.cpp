#include "../src/stack_vm.hpp"
#include <iostream>
#include <cassert>
#include <vector>

void test_load_store() {
    std::cout << "Testing LOAD and STORE..." << std::endl;

    StackVM vm;
    std::vector<uint64_t> program = {
        // Store 42 at address 16384 (heap start)
        static_cast<uint64_t>(Opcode::PUSH), 42,
        static_cast<uint64_t>(Opcode::PUSH), 16384,
        static_cast<uint64_t>(Opcode::STORE),

        // Load from address 16384
        static_cast<uint64_t>(Opcode::PUSH), 16384,
        static_cast<uint64_t>(Opcode::LOAD),
        static_cast<uint64_t>(Opcode::HALT)
    };

    vm.load_program(program);
    vm.execute();

    assert(vm.get_top() == 42);
    std::cout << "  ✓ STORE/LOAD: Store 42 at 16384, load back -> 42" << std::endl;
}

void test_multiple_stores() {
    std::cout << "Testing multiple STORE operations..." << std::endl;

    StackVM vm;
    std::vector<uint64_t> program = {
        // Store 100 at 16384
        static_cast<uint64_t>(Opcode::PUSH), 100,
        static_cast<uint64_t>(Opcode::PUSH), 16384,
        static_cast<uint64_t>(Opcode::STORE),

        // Store 200 at 16385
        static_cast<uint64_t>(Opcode::PUSH), 200,
        static_cast<uint64_t>(Opcode::PUSH), 16385,
        static_cast<uint64_t>(Opcode::STORE),

        // Store 300 at 16386
        static_cast<uint64_t>(Opcode::PUSH), 300,
        static_cast<uint64_t>(Opcode::PUSH), 16386,
        static_cast<uint64_t>(Opcode::STORE),

        // Load all three and verify last one
        static_cast<uint64_t>(Opcode::PUSH), 16384,
        static_cast<uint64_t>(Opcode::LOAD),
        static_cast<uint64_t>(Opcode::PUSH), 16385,
        static_cast<uint64_t>(Opcode::LOAD),
        static_cast<uint64_t>(Opcode::PUSH), 16386,
        static_cast<uint64_t>(Opcode::LOAD),
        static_cast<uint64_t>(Opcode::HALT)
    };

    vm.load_program(program);
    vm.execute();

    assert(vm.get_top() == 300);
    std::cout << "  ✓ Multiple stores work correctly" << std::endl;
}

void test_store_code_segment_protection() {
    std::cout << "Testing code segment write protection..." << std::endl;

    StackVM vm;
    std::vector<uint64_t> program = {
        // Try to write to code segment (address < 16384)
        static_cast<uint64_t>(Opcode::PUSH), 42,
        static_cast<uint64_t>(Opcode::PUSH), 100,  // Code segment
        static_cast<uint64_t>(Opcode::STORE),
        static_cast<uint64_t>(Opcode::HALT)
    };

    vm.load_program(program);

    bool caught = false;
    try {
        vm.execute();
    } catch (const std::runtime_error& e) {
        caught = true;
        std::string msg(e.what());
        assert(msg.find("code segment") != std::string::npos);
    }

    assert(caught);
    std::cout << "  ✓ Code segment write protection works" << std::endl;
}

void test_load_bounds_check() {
    std::cout << "Testing LOAD bounds checking..." << std::endl;

    StackVM vm;
    std::vector<uint64_t> program = {
        // Try to load from out-of-bounds address
        static_cast<uint64_t>(Opcode::PUSH), 100000,  // Out of bounds
        static_cast<uint64_t>(Opcode::LOAD),
        static_cast<uint64_t>(Opcode::HALT)
    };

    vm.load_program(program);

    bool caught = false;
    try {
        vm.execute();
    } catch (const std::runtime_error& e) {
        caught = true;
        std::string msg(e.what());
        assert(msg.find("bounds") != std::string::npos);
    }

    assert(caught);
    std::cout << "  ✓ LOAD bounds checking works" << std::endl;
}

void test_store_bounds_check() {
    std::cout << "Testing STORE bounds checking..." << std::endl;

    StackVM vm;
    std::vector<uint64_t> program = {
        // Try to store to out-of-bounds address
        static_cast<uint64_t>(Opcode::PUSH), 42,
        static_cast<uint64_t>(Opcode::PUSH), 100000,  // Out of bounds
        static_cast<uint64_t>(Opcode::STORE),
        static_cast<uint64_t>(Opcode::HALT)
    };

    vm.load_program(program);

    bool caught = false;
    try {
        vm.execute();
    } catch (const std::runtime_error& e) {
        caught = true;
        std::string msg(e.what());
        assert(msg.find("bounds") != std::string::npos);
    }

    assert(caught);
    std::cout << "  ✓ STORE bounds checking works" << std::endl;
}

void test_memory_as_array() {
    std::cout << "Testing memory as array..." << std::endl;

    StackVM vm;
    std::vector<uint64_t> program = {
        // Initialize array at 16400
        static_cast<uint64_t>(Opcode::PUSH), 10,
        static_cast<uint64_t>(Opcode::PUSH), 16400,
        static_cast<uint64_t>(Opcode::STORE),

        static_cast<uint64_t>(Opcode::PUSH), 20,
        static_cast<uint64_t>(Opcode::PUSH), 16401,
        static_cast<uint64_t>(Opcode::STORE),

        static_cast<uint64_t>(Opcode::PUSH), 30,
        static_cast<uint64_t>(Opcode::PUSH), 16402,
        static_cast<uint64_t>(Opcode::STORE),

        // Read array[1]
        static_cast<uint64_t>(Opcode::PUSH), 16401,
        static_cast<uint64_t>(Opcode::LOAD),
        static_cast<uint64_t>(Opcode::HALT)
    };

    vm.load_program(program);
    vm.execute();

    assert(vm.get_top() == 20);
    std::cout << "  ✓ Memory can be used as array" << std::endl;
}

void test_read_write_cycle() {
    std::cout << "Testing read-modify-write cycle..." << std::endl;

    StackVM vm;
    std::vector<uint64_t> program = {
        // Store initial value
        static_cast<uint64_t>(Opcode::PUSH), 10,
        static_cast<uint64_t>(Opcode::PUSH), 16500,
        static_cast<uint64_t>(Opcode::STORE),

        // Load value
        static_cast<uint64_t>(Opcode::PUSH), 16500,
        static_cast<uint64_t>(Opcode::LOAD),

        // Modify (add 5)
        static_cast<uint64_t>(Opcode::PUSH), 5,
        static_cast<uint64_t>(Opcode::ADD),

        // Store back
        static_cast<uint64_t>(Opcode::PUSH), 16500,
        static_cast<uint64_t>(Opcode::STORE),

        // Load again to verify
        static_cast<uint64_t>(Opcode::PUSH), 16500,
        static_cast<uint64_t>(Opcode::LOAD),
        static_cast<uint64_t>(Opcode::HALT)
    };

    vm.load_program(program);
    vm.execute();

    assert(vm.get_top() == 15);
    std::cout << "  ✓ Read-modify-write cycle: 10 + 5 = 15" << std::endl;
}

int main() {
    std::cout << "=== VM Memory Operations Tests ===" << std::endl;

    try {
        test_load_store();
        test_multiple_stores();
        test_store_code_segment_protection();
        test_load_bounds_check();
        test_store_bounds_check();
        test_memory_as_array();
        test_read_write_cycle();

        std::cout << "\n✓ All memory operation tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Test failed: " << e.what() << std::endl;
        return 1;
    }
}
