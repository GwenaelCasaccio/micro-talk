#include "../src/stack_vm.hpp"
#include <cassert>
#include <csignal>
#include <iostream>
#include <vector>

// Test that CLI disables interrupts
void test_cli() {
    std::cout << "Testing CLI (disable interrupts)..." << '\n';

    StackVM vm;

    std::vector<uint64_t> program = {static_cast<uint64_t>(Opcode::CLI),
                                     static_cast<uint64_t>(Opcode::HALT)};

    vm.load_program(program);
    vm.execute();

    // Verify interrupts are disabled
    assert(!vm.interrupts_enabled());

    std::cout << "  ✓ CLI disables interrupts" << '\n';
}

// Test that STI enables interrupts
void test_sti() {
    std::cout << "Testing STI (enable interrupts)..." << '\n';

    StackVM vm;

    std::vector<uint64_t> program = {static_cast<uint64_t>(Opcode::CLI),
                                     static_cast<uint64_t>(Opcode::STI),
                                     static_cast<uint64_t>(Opcode::HALT)};

    vm.load_program(program);
    vm.execute();

    // Verify interrupts are enabled after STI
    assert(vm.interrupts_enabled());

    std::cout << "  ✓ STI enables interrupts" << '\n';
}

// Test that IRET returns from handler and re-enables interrupts
void test_iret() {
    std::cout << "Testing IRET (interrupt return)..." << '\n';

    StackVM vm;

    std::vector<uint64_t> program = {static_cast<uint64_t>(Opcode::PUSH),
                                     7, // Return address
                                     static_cast<uint64_t>(Opcode::CLI),
                                     static_cast<uint64_t>(Opcode::PUSH),
                                     42,
                                     static_cast<uint64_t>(Opcode::POP),
                                     static_cast<uint64_t>(Opcode::IRET),
                                     static_cast<uint64_t>(Opcode::PUSH),
                                     99,
                                     static_cast<uint64_t>(Opcode::HALT)};

    vm.load_program(program);
    vm.execute();

    // Verify interrupts are re-enabled after IRET
    assert(vm.interrupts_enabled());

    // Verify we continued execution (99 should be on stack)
    assert(vm.get_top() == 99);

    std::cout << "  ✓ IRET returns and re-enables interrupts" << '\n';
}

// Test SIGNAL_REG for registering and unregistering handlers
void test_signal_reg() {
    std::cout << "Testing SIGNAL_REG..." << '\n';

    StackVM vm;

    std::vector<uint64_t> program = {
        // Register handler
        static_cast<uint64_t>(Opcode::PUSH), 100, static_cast<uint64_t>(Opcode::PUSH),
        static_cast<uint64_t>(SIGUSR1), static_cast<uint64_t>(Opcode::SIGNAL_REG),
        static_cast<uint64_t>(Opcode::HALT),

        // Unregister handler
        static_cast<uint64_t>(Opcode::PUSH), 0, static_cast<uint64_t>(Opcode::PUSH),
        static_cast<uint64_t>(SIGUSR1), static_cast<uint64_t>(Opcode::SIGNAL_REG),

        static_cast<uint64_t>(Opcode::HALT)};

    vm.load_program(program);
    vm.execute();

    assert(vm.get_signal_handler(SIGUSR1) == 100);

    vm.execute();
    assert(vm.get_signal_handler(SIGUSR1) == 0);

    std::cout << "  ✓ SIGNAL_REG registers and unregisters handlers" << '\n';
}

// Test automatic signal handler invocation
void test_signal_handling() {
    std::cout << "Testing automatic signal handling..." << '\n';

    StackVM vm;

    std::vector<uint64_t> program = {
        // Main code
        static_cast<uint64_t>(Opcode::PUSH), 11, static_cast<uint64_t>(Opcode::PUSH),
        static_cast<uint64_t>(SIGUSR1), static_cast<uint64_t>(Opcode::SIGNAL_REG),
        static_cast<uint64_t>(Opcode::PUSH), 0, // Marker
        static_cast<uint64_t>(Opcode::HALT),    // Register events => interrup are ready
        static_cast<uint64_t>(Opcode::HALT),

        // Padding to reach address 10
        0, 0,

        static_cast<uint64_t>(Opcode::PUSH), 42, static_cast<uint64_t>(Opcode::PUSH), 16500,
        static_cast<uint64_t>(Opcode::STORE), static_cast<uint64_t>(Opcode::IRET)};

    vm.load_program(program);

    vm.execute();

    std::raise(SIGUSR1);

    vm.execute();

    assert(vm.read_memory(16500) == 42);

    std::cout << "  ✓ Signal handler called automatically" << '\n';
}

// Test that interrupts are disabled during handler execution
void test_interrupt_disable_during_handler() {
    std::cout << "Testing interrupt disable during handler..." << '\n';

    StackVM vm;

    std::vector<uint64_t> program = {
        static_cast<uint64_t>(Opcode::PUSH), 10, static_cast<uint64_t>(Opcode::PUSH),
        static_cast<uint64_t>(SIGUSR1), static_cast<uint64_t>(Opcode::SIGNAL_REG),
        static_cast<uint64_t>(Opcode::HALT), static_cast<uint64_t>(Opcode::HALT),

        // Padding
        0, 0, 0,

        static_cast<uint64_t>(Opcode::HALT), static_cast<uint64_t>(Opcode::PUSH), 16500,
        static_cast<uint64_t>(Opcode::LOAD), static_cast<uint64_t>(Opcode::PUSH), 1,
        static_cast<uint64_t>(Opcode::ADD), static_cast<uint64_t>(Opcode::PUSH), 16500,
        static_cast<uint64_t>(Opcode::STORE), static_cast<uint64_t>(Opcode::HALT),
        static_cast<uint64_t>(Opcode::IRET)};

    vm.load_program(program);
    vm.execute();

    std::raise(SIGUSR1);

    vm.execute();

    assert(!vm.interrupts_enabled());

    std::raise(SIGUSR1);

    vm.execute();

    vm.execute();

    assert(!vm.interrupts_enabled());
    assert(vm.read_memory(16500) == 1);

    std::cout << "  ✓ Interrupts disabled during handler, re-enabled after IRET" << '\n';
}

// Test multiple signals with different handlers
void test_multiple_signal_handlers() {
    std::cout << "Testing multiple signal handlers..." << '\n';

    StackVM vm;

    std::vector<uint64_t> program = {static_cast<uint64_t>(Opcode::HALT),
                                     static_cast<uint64_t>(Opcode::PUSH),
                                     12,
                                     static_cast<uint64_t>(Opcode::PUSH),
                                     static_cast<uint64_t>(SIGUSR1),
                                     static_cast<uint64_t>(Opcode::SIGNAL_REG),

                                     static_cast<uint64_t>(Opcode::PUSH),
                                     18,
                                     static_cast<uint64_t>(Opcode::PUSH),
                                     static_cast<uint64_t>(SIGUSR2),
                                     static_cast<uint64_t>(Opcode::SIGNAL_REG),

                                     static_cast<uint64_t>(Opcode::HALT),

                                     static_cast<uint64_t>(Opcode::PUSH),
                                     100,
                                     static_cast<uint64_t>(Opcode::PUSH),
                                     16500,
                                     static_cast<uint64_t>(Opcode::STORE),
                                     static_cast<uint64_t>(Opcode::IRET),

                                     static_cast<uint64_t>(Opcode::PUSH),
                                     200,
                                     static_cast<uint64_t>(Opcode::PUSH),
                                     16600,
                                     static_cast<uint64_t>(Opcode::STORE),
                                     static_cast<uint64_t>(Opcode::IRET)};

    vm.load_program(program);

    vm.execute();

    std::raise(SIGUSR1);
    std::raise(SIGUSR2);

    vm.execute();

    assert(vm.read_memory(16500) == 100);
    assert(vm.read_memory(16600) == 200);

    std::cout << "  ✓ Multiple signal handlers work correctly" << '\n';
}

int main() {
    std::cout << "=== VM Interrupt Tests ===" << '\n' << '\n';

    try {
        test_cli();
        test_sti();
        test_iret();
        test_signal_reg();
        test_signal_handling();
        test_interrupt_disable_during_handler();
        test_multiple_signal_handlers();

        std::cout << "\n✓ All interrupt tests passed!" << '\n';
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Test failed: " << e.what() << '\n';
        return 1;
    }
}
