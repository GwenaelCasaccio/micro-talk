#include "../src/lisp_compiler.hpp"
#include "../src/lisp_parser.hpp"
#include "../src/stack_vm.hpp"
#include <cassert>
#include <iostream>

void test_basic_checkpoint_restore() {
    std::cout << "Test: Basic checkpoint and restore" << std::endl;

    StackVM vm;
    LispCompiler compiler;

    // Compile and run: (define-var x 42)
    LispParser parser1("(do (define-var x 42) x)");
    auto ast1 = parser1.parse();
    auto program1 = compiler.compile(ast1);
    vm.load_program(program1);
    vm.execute();

    // Verify initial state
    uint64_t result1 = vm.get_top();
    std::cout << "  After (define-var x 42): " << result1 << std::endl;
    assert(result1 == 42);

    // Save checkpoint
    auto snapshot = vm.checkpoint();
    std::cout << "  Checkpoint saved (x=42)" << std::endl;

    // Modify state: (set x 100)
    compiler.reset();
    vm.reset();
    LispParser parser2("(do (define-var x 42) (set x 100) x)");
    auto ast2 = parser2.parse();
    auto program2 = compiler.compile(ast2);
    vm.load_program(program2);
    vm.execute();

    uint64_t result2 = vm.get_top();
    std::cout << "  After (set x 100): " << result2 << std::endl;
    assert(result2 == 100);

    // Restore to checkpoint
    vm.restore(snapshot);
    std::cout << "  Restored to checkpoint" << std::endl;

    // Verify state was restored
    uint64_t restored = vm.get_top();
    std::cout << "  Value after restore: " << restored << std::endl;
    assert(restored == 42);

    std::cout << "  ✓ Basic checkpoint/restore test passed" << std::endl;
}

void test_stack_checkpoint() {
    std::cout << "\nTest: Stack state preservation" << std::endl;

    StackVM vm;
    LispCompiler compiler;

    // Build up stack: compute (+ (* 5 3) (* 2 4))
    LispParser parser1("(+ (* 5 3) (* 2 4))");
    auto ast = parser1.parse();
    auto program = compiler.compile(ast);
    vm.load_program(program);
    vm.execute();

    uint64_t result = vm.get_top();
    uint64_t sp_before = vm.get_sp();
    std::cout << "  Computed: " << result << " (SP=" << sp_before << ")" << std::endl;
    assert(result == 23); // 15 + 8

    // Save checkpoint
    auto snapshot = vm.checkpoint();

    // Do more computation: (+ 1000 2000)
    compiler.reset();
    vm.reset();
    LispParser parser2("(+ 1000 2000)");
    auto ast2 = parser2.parse();
    auto program2 = compiler.compile(ast2);
    vm.load_program(program2);
    vm.execute();

    uint64_t result2 = vm.get_top();
    uint64_t sp_after = vm.get_sp();
    std::cout << "  New computation: " << result2 << " (SP=" << sp_after << ")" << std::endl;
    assert(result2 == 3000);

    // Restore
    vm.restore(snapshot);

    // Verify everything restored
    uint64_t restored_result = vm.get_top();
    uint64_t restored_sp = vm.get_sp();
    std::cout << "  Restored: " << restored_result << " (SP=" << restored_sp << ")" << std::endl;
    assert(restored_result == 23);
    assert(restored_sp == sp_before);

    std::cout << "  ✓ Stack preservation test passed" << std::endl;
}

void test_multiple_checkpoints() {
    std::cout << "\nTest: Multiple checkpoints" << std::endl;

    StackVM vm;
    LispCompiler compiler;

    // State 1: x = 10
    LispParser parser1("(do (define-var x 10) x)");
    auto ast1 = parser1.parse();
    auto program1 = compiler.compile(ast1);
    vm.load_program(program1);
    vm.execute();

    auto checkpoint1 = vm.checkpoint();
    std::cout << "  Checkpoint 1: x=10" << std::endl;

    // State 2: x = 20
    compiler.reset();
    vm.reset();
    LispParser parser2("(do (define-var x 10) (set x 20) x)");
    auto ast2 = parser2.parse();
    auto program2 = compiler.compile(ast2);
    vm.load_program(program2);
    vm.execute();

    auto checkpoint2 = vm.checkpoint();
    std::cout << "  Checkpoint 2: x=20" << std::endl;

    // State 3: x = 30
    compiler.reset();
    vm.reset();
    LispParser parser3("(do (define-var x 10) (set x 30) x)");
    auto ast3 = parser3.parse();
    auto program3 = compiler.compile(ast3);
    vm.load_program(program3);
    vm.execute();

    std::cout << "  Current state: x=30 (result=" << vm.get_top() << ")" << std::endl;
    assert(vm.get_top() == 30);

    // Restore to checkpoint 2
    vm.restore(checkpoint2);
    std::cout << "  Restored to checkpoint 2: " << vm.get_top() << std::endl;
    assert(vm.get_top() == 20);

    // Restore to checkpoint 1
    vm.restore(checkpoint1);
    std::cout << "  Restored to checkpoint 1: " << vm.get_top() << std::endl;
    assert(vm.get_top() == 10);

    // Restore to checkpoint 2 again
    vm.restore(checkpoint2);
    std::cout << "  Restored to checkpoint 2 again: " << vm.get_top() << std::endl;
    assert(vm.get_top() == 20);

    std::cout << "  ✓ Multiple checkpoints test passed" << std::endl;
}

void test_memory_checkpoint() {
    std::cout << "\nTest: Memory state preservation" << std::endl;

    StackVM vm;
    LispCompiler compiler;

    // Run a simple computation to get VM in a known state
    LispParser parser1("(+ 100 200)");
    auto ast = parser1.parse();
    auto program = compiler.compile(ast);
    vm.load_program(program);
    vm.execute();

    assert(vm.get_top() == 300);

    // Write some test data to heap memory (must be >= HEAP_START = 268435456)
    const uint64_t heap_addr1 = 268435456; // First word of heap
    const uint64_t heap_addr2 = 268435457; // Second word of heap

    vm.write_memory(heap_addr1, 42);
    vm.write_memory(heap_addr2, 99);
    std::cout << "  Initial: Memory[heap]=" << vm.read_memory(heap_addr1)
              << ", Memory[heap+1]=" << vm.read_memory(heap_addr2) << std::endl;

    // Save checkpoint
    auto snapshot = vm.checkpoint();
    std::cout << "  Checkpoint saved" << std::endl;

    // Modify memory (simulating state changes)
    vm.write_memory(heap_addr1, 111);
    vm.write_memory(heap_addr2, 222);

    assert(vm.read_memory(heap_addr1) == 111);
    assert(vm.read_memory(heap_addr2) == 222);
    std::cout << "  Modified: Memory[heap]=" << vm.read_memory(heap_addr1)
              << ", Memory[heap+1]=" << vm.read_memory(heap_addr2) << std::endl;

    // Restore from checkpoint
    vm.restore(snapshot);

    // Verify all state was restored
    assert(vm.read_memory(heap_addr1) == 42);
    assert(vm.read_memory(heap_addr2) == 99);
    assert(vm.get_top() == 300); // Stack should also be restored
    std::cout << "  Restored: Memory[heap]=" << vm.read_memory(heap_addr1)
              << ", Memory[heap+1]=" << vm.read_memory(heap_addr2) << ", Stack=" << vm.get_top()
              << std::endl;

    std::cout << "  ✓ Memory preservation test passed" << std::endl;
}

int main() {
    std::cout << "=== VM Checkpoint/Restore Tests ===" << std::endl;
    std::cout << std::endl;

    test_basic_checkpoint_restore();
    test_stack_checkpoint();
    test_multiple_checkpoints();
    test_memory_checkpoint();

    std::cout << "\n=== All checkpoint tests passed! ===" << std::endl;
    return 0;
}
