#include <iostream>
#include <vector>
#include <cstdint>

int main() {
    // Simulate the stack operations
    std::vector<uint64_t> stack;
    size_t sp = 0;  // Stack grows down, sp points to next free slot
    size_t bp = 0;
    
    auto push = [&](uint64_t v) { stack.push_back(v); sp++; };
    auto pop = [&]() { sp--; uint64_t v = stack.back(); stack.pop_back(); return v; };
    
    std::cout << "Initial: sp=" << sp << " bp=" << bp << std::endl;
    
    // Push argument
    push(42);
    std::cout << "After push arg: sp=" << sp << " bp=" << bp << " stack=[42]" << std::endl;
    
    // CALL
    push(7);  // return address
    push(bp); // old BP (0)
    bp = sp;  // BP = current SP
    std::cout << "After CALL: sp=" << sp << " bp=" << bp << " stack=[42,7,0]" << std::endl;
    
    // Store parameter (pop arg, pop address, store - but pops happen)
    //uint64_t param_addr = ...; 
    // Let's just skip param handling and go to body
    
    // Function body pushes result
    push(99);
    std::cout << "After body: sp=" << sp << " bp=" << bp << " stack=[42,7,0,99]" << std::endl;
    
    // RET
    uint64_t result = pop();
    std::cout << "Popped result: " << result << " sp=" << sp << std::endl;
    
    sp = bp;  // This should restore to where we were after CALL
    std::cout << "After sp=bp: sp=" << sp << " (stack size=" << stack.size() << ")" << std::endl;
    
    bp = pop();
    std::cout << "Popped old bp: " << bp << " sp=" << sp << std::endl;
    
    uint64_t ret_addr = pop();
    std::cout << "Popped ret_addr: " << ret_addr << " sp=" << sp << std::endl;
    
    push(result);
    std::cout << "Pushed result: sp=" << sp << std::endl;
    
    std::cout << "Final stack top: " << stack.back() << std::endl;
    
    return 0;
}
