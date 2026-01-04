#include <cstdint>
#include <iostream>
#include <vector>

int main() {
    // Simulate the stack operations
    std::vector<uint64_t> stack;
    size_t sp = 0; // Stack grows down, sp points to next free slot
    size_t bp = 0;

    auto push = [&](uint64_t v) {
        stack.push_back(v);
        sp++;
    };
    auto pop = [&]() {
        sp--;
        uint64_t v = stack.back();
        stack.pop_back();
        return v;
    };

    std::cout << "Initial: sp=" << sp << " bp=" << bp << '\n';

    // Push argument
    push(42);
    std::cout << "After push arg: sp=" << sp << " bp=" << bp << " stack=[42]" << '\n';

    // CALL
    push(7);  // return address
    push(bp); // old BP (0)
    bp = sp;  // BP = current SP
    std::cout << "After CALL: sp=" << sp << " bp=" << bp << " stack=[42,7,0]" << '\n';

    // Store parameter (pop arg, pop address, store - but pops happen)
    // uint64_t param_addr = ...;
    // Let's just skip param handling and go to body

    // Function body pushes result
    push(99);
    std::cout << "After body: sp=" << sp << " bp=" << bp << " stack=[42,7,0,99]" << '\n';

    // RET
    uint64_t result = pop();
    std::cout << "Popped result: " << result << " sp=" << sp << '\n';

    sp = bp; // This should restore to where we were after CALL
    std::cout << "After sp=bp: sp=" << sp << " (stack size=" << stack.size() << ")" << '\n';

    bp = pop();
    std::cout << "Popped old bp: " << bp << " sp=" << sp << '\n';

    uint64_t ret_addr = pop();
    std::cout << "Popped ret_addr: " << ret_addr << " sp=" << sp << '\n';

    push(result);
    std::cout << "Pushed result: sp=" << sp << '\n';

    std::cout << "Final stack top: " << stack.back() << '\n';

    return 0;
}
