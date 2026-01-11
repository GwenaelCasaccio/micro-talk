# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

A minimal 64-bit stack-based virtual machine with a Lisp compiler and Smalltalk-style object system. The VM features a unified flat memory model, extensible microcode system, and tagged pointer architecture for building higher-level languages.

## Build and Test Commands

### Building

```bash
make                 # Build all binaries (VM + all tests)
make clean          # Remove all build artifacts
```

### Running

```bash
make run            # Run the main REPL (./build/lisp_vm)
./build/lisp_vm     # Direct execution - interactive Lisp REPL
```

### Testing

**Run All Tests:**
```bash
make test-all       # Run all tests (unit + integration)
```

**VM Core Tests** (tests/ directory):
```bash
make vm-stack       # Stack operations (PUSH, POP, DUP, SWAP, underflow)
make vm-alu         # ALU: arithmetic, comparison, bitwise operations
make vm-memory      # Memory: LOAD, STORE, bounds checking, protection
make vm-control     # Control flow: JMP, JZ, conditionals
make vm-all         # Run all VM tests together
```

**Parser Tests** (tests/ directory):
```bash
make parser-basic   # Basic parsing: numbers, symbols, strings, lists
make parser-comments # Comment handling in all contexts
make parser-errors  # Error detection and handling
make parser-all     # Run all parser tests
```

**Compiler Tests** (tests/ directory):
```bash
make compiler-basic     # Numbers, arithmetic, comparison, bitwise, nested expressions
make compiler-control   # If-then-else, while/for loops, nested control flow
make compiler-variables # Define, set, let bindings, shadowing, variable arithmetic
make compiler-functions # Functions, parameters, calls, conditionals (recursion in src/)
make compiler-all       # Run all compiler tests
```

**Transpiler Tests** (tests/ directory):
```bash
make transpiler         # Lisp-to-C++ transpiler tests (24 tests)
```

**Integration Tests** (src/ directory):
```bash
make integration-all # Run all integration tests
make test           # Run tagging tests (./build/test_tagging)
make simple         # Run simple tagging tests (./build/simple_tagging_test)
make vars           # Run variable tests (./build/test_variables)
make lambda         # Run lambda tests (./build/test_lambda)
make loops          # Run loop tests (./build/test_loops)
make micro          # Run microcode tests (./build/test_microcode)
make advanced       # Run advanced tests (./build/test_advanced)
make smalltalk      # Run Smalltalk object model tests (./build/test_smalltalk)
make comments       # Run comment parsing tests (./build/test_comments)
```

**Performance Benchmarking:**
```bash
make benchmark      # Run dispatch benchmarks (-O0 and -O3)
make benchmark-o0   # Benchmark at -O0 (debug build)
make benchmark-o3   # Benchmark at -O3 (release build, 2.8-3.6x faster)
```

Each test target builds and runs the corresponding test binary.

**Test Summary:**
- Total: 169+ tests across all suites
- Unit tests: 169 tests (VM: 43, Parser: 58, Compiler: 44, Transpiler: 24)
- Integration tests: 9 test suites covering end-to-end functionality

## Architecture

### Multi-Tier System

1. **Stack VM (stack_vm.hpp)** - Low-level bytecode execution engine with mmap'd memory
2. **Lisp Compiler (lisp_compiler.hpp)** - Compiles Lisp to VM bytecode
3. **Lisp-to-C++ Transpiler (lisp_to_cpp.hpp)** - Transpiles Lisp to standalone C++ code
4. **Microcode System (microcode.hpp)** - Extends VM with Lisp-defined instructions

### Dispatch Optimization (Computed Goto)

The VM uses **computed goto** (GCC/Clang labels-as-values extension) for instruction dispatch, providing significant performance improvements over traditional switch-based dispatch:

**Performance Characteristics:**
- **2.8-3.6x faster** than switch statement at -O3 optimization
- Direct jump table eliminates branch misprediction overhead
- Better instruction cache locality
- Benchmark results (1M arithmetic operations):
  - Debug (-O0): ~120ms
  - Release (-O3): ~43ms (2.84x speedup)

**Implementation Details:**
- Jump table with 43 opcode labels (`&&op_halt`, `&&op_push`, etc.)
- Dispatch: `goto *dispatch_table[opcode_idx]`
- Each handler ends with `continue` to re-enter main loop
- Requires GCC or Clang compiler (non-portable)

**Compiler Requirement:**
- Built with GCC or Clang (standard on Linux/macOS)
- Uses `-std=c++17 -O3` for maximum performance
- Use `make OPTIMIZE=1` for release builds with full optimization

### Memory Model

Single flat 64K-word (512KB) mmap-allocated memory space:

```
[0 ... 16384)          Code segment (128KB, write-protected during execution)
[16384 ... 65536)      Heap (grows upward from 16384)
[65536 ... 65536)      Stack (grows downward from 65536)
```

**Key registers:**
- `IP` (Instruction Pointer): Current instruction in code segment
- `SP` (Stack Pointer): Top of stack, grows downward from 65536
- `BP` (Base Pointer): Frame pointer for function calls
- `HP` (Heap Pointer): Next free heap address, grows upward from 16384

**Safety features:**
- Code segment write protection during execution
- Stack overflow detection (collision with heap at SP <= HP)
- Bounds checking on all memory access

### Tagged Pointer System

Lower 3 bits encode type (heap addresses are 8-byte aligned):

```
000 - OOP (Ordinary Object Pointer - heap address)
001 - INT (Small integer, 61-bit signed)
111 - SPECIAL (nil, true, false - reserved)
```

**Tag operations (Lisp):**
- `(bit-or (bit-shl value 3) 1)` - Tag integer
- `(bit-ashr tagged 3)` - Untag integer (arithmetic shift preserves sign)
- `(bit-and tagged 7)` - Extract tag bits
- `(= (bit-and obj 7) 0)` - Check if OOP
- `(= (bit-and obj 7) 1)` - Check if INT

### Smalltalk Object Model

**Class structure (3 words):**
- Offset 0: Class name (tagged int)
- Offset 1: Superclass (OOP or NULL)
- Offset 2: Named slot count (tagged int)

**Instance structure:**
- Offset 0: Class (OOP)
- Offset 1: Shape (0=fixed, 1=indexable/array, 2=bytes)
- Offset 2: Named slot count
- Offset 3+: Named slots
- After named slots: Indexed slots (for SHAPE_INDEXABLE)

**API in Lisp files (see lisp/smalltalk.lisp):**
- `(new-class name superclass slot-count)` - Create class
- `(new-instance class named indexed shape)` - Create instance
- `(slot-at obj index)` / `(slot-at-put obj index value)` - Named slot access
- `(indexed-at obj index)` / `(indexed-at-put obj index value)` - Array access

### Microcode System

Extends VM with Lisp-defined instructions (opcodes 100-255):

```lisp
(defmicro square (x) (* x x))
```

Compiles Lisp code into bytecode and registers it with a unique opcode. Used to implement:
- Smalltalk primitives (object creation, message send)
- Tagged arithmetic operations
- Domain-specific operations

Microcode functions are compiled once and reusable as VM primitives.

## File Organization

### Core VM Components
- `src/stack_vm.hpp` - VM execution engine, opcode handlers, memory management
- `src/lisp_parser.hpp` - S-expression parser (creates AST from text)
- `src/lisp_compiler.hpp` - Lisp→bytecode compiler with scoping, functions, loops
- `src/microcode.hpp` - Microcode definition and compilation system

### Main Entry Points
- `src/main.cpp` - Interactive REPL for Lisp expressions
- `src/test_*.cpp` - Individual test programs for specific features

### Lisp Programs
- `lisp/` - Example Lisp programs demonstrating features
- `lisp/smalltalk.lisp` - Complete Smalltalk object model implementation
- `lisp/allocator_*.lisp` - Memory allocation examples
- `lisp/tagging*.lisp` - Tagged pointer demonstrations

### Documentation
- `docs/README.md` - High-level architecture overview
- `docs/TAGGING.md` - Tagged pointer system details
- `docs/MICROCODE.md` - Microcode system guide
- `docs/SMALLTALK.md` - Smalltalk object model specification
- `docs/VARIABLES.md`, `LAMBDA.md`, `LOOPS.md` - Language feature docs

## Instruction Set

### Stack Operations
`PUSH <imm>`, `POP`, `DUP`, `SWAP`

### Arithmetic
`ADD`, `SUB`, `MUL`, `DIV`, `MOD`

### Comparison
`EQ`, `LT`, `GT`

### Control Flow
`JMP <addr>`, `JZ <addr>`, `CALL <addr>`, `RET`

### Memory
`LOAD`, `STORE`

### Bitwise (for tagged pointers)
`AND`, `OR`, `XOR`, `SHL`, `SHR`, `ASHR` (arithmetic shift)

### Debug
`PRINT`, `PRINT_STR`, `HALT`

## Lisp Language Features

### Comments

Line comments start with `;` and continue to end of line:

```lisp
; This is a comment
(+ 5 3)  ; Inline comment after expression

; Multiple comments
; can be placed
; on consecutive lines
```

### Supported Forms
- Arithmetic: `(+ a b ...)`, `(- a b ...)`, `(* a b ...)`, `(/ a b ...)`, `(% a b)`
- Comparison: `(= a b)`, `(< a b)`, `(> a b)`
- Bitwise: `(bit-and a b)`, `(bit-or a b)`, `(bit-xor a b)`, `(bit-shl a n)`, `(bit-shr a n)`, `(bit-ashr a n)`
- Control: `(if cond then else)`, `(do expr1 expr2 ...)` (sequential, returns last)
- Loops: `(while cond body...)`, `(for (var start end) body...)`
- Variables: `(define var value)`, `(set var value)`, `(let ((var val)...) body...)`
- Functions: `(define (name params...) body)`
- Memory: `(peek addr)`, `(poke addr value)`
- Debug: `(print expr)`, `(print-string expr)`

### Variable Scoping
- Lexical scoping with nested environments
- Variables stored in heap memory starting at address 16384
- `define` creates new binding in current scope
- `set` modifies existing binding (searches parent scopes)
- `let` creates new scope for bindings

### Functions
Function calls use stack-based calling convention:
1. Push arguments (left to right)
2. CALL pushes return address and jumps
3. Function pops return address, then arguments (into local variables)
4. Function evaluates body, result on stack
5. RET swaps result with return address and jumps back

## Common Development Patterns

### Adding New VM Instructions

1. Add opcode to enum in `src/stack_vm.hpp`
2. Add case handler in `StackVM::execute()` switch statement
3. Update `src/lisp_compiler.hpp` to emit new opcode (if needed for Lisp)

### Adding Lisp Language Features

Edit `src/lisp_compiler.hpp`:
- Add parsing logic in `compile_expr()` for new symbol/form
- Create helper method (e.g., `compile_while()`, `compile_for()`)
- Emit appropriate bytecode sequence

### Creating Test Programs

1. Create `src/test_<feature>.cpp` with main() that creates VM, compiles code, executes
2. Add test target to Makefile following existing pattern
3. Add corresponding make command (e.g., `make feature`)

### Working with Tagged Values in Lisp

Always tag integers before storing, untag before arithmetic:

```lisp
(define x (tag-int 42))           ; Tag for storage
(define y (untag-int x))          ; Untag for arithmetic
(define z (+ y 10))               ; Operate on untagged
(define result (tag-int z))       ; Tag result
```

For OOP values (heap addresses):
```lisp
(define obj-addr 16384)           ; Already 8-byte aligned
(define tagged-oop (bit-or obj-addr 0))  ; Tag as OOP (no-op for aligned addresses)
(define addr (bit-and tagged-oop (bit-xor -1 7)))  ; Untag OOP
```

## Compiler Internals

### Bytecode Emission
- `emit(uint64_t)` - Append value to bytecode vector
- `emit_opcode(Opcode)` - Append opcode
- `current_address()` - Get current bytecode position (for jump patching)

### Jump Patching
For control flow, emit placeholder address then patch:

```cpp
emit_opcode(Opcode::JZ);
size_t jump_pos = current_address();
emit(0);  // Placeholder
// ... compile branch ...
bytecode[jump_pos] = current_address();  // Patch jump target
```

### Function Compilation
- First pass: Record function definitions in `functions` map
- Main code: Emit CALL with placeholder address, store in `label_refs`
- After main: `compile_all_functions()` emits function bodies, records addresses in `labels`
- Final: `patch_function_calls()` fills in CALL targets from `labels`

## Lisp-to-C++ Transpiler

The transpiler (`src/lisp_to_cpp.hpp`) converts Lisp code to standalone C++ programs that can be compiled and run independently of the VM.

### Features Supported

**Arithmetic & Comparison:**
- All arithmetic operations: `+`, `-`, `*`, `/`, `%` (multi-argument support for +, -, *)
- Comparison: `=`, `<`, `>`, `<=`, `>=`
- Bitwise: `bit-and`, `bit-or`, `bit-xor`, `bit-shl`, `bit-shr`, `bit-ashr`

**Variables:**
- `(define var value)` - Variable definition
- `(set var value)` - Variable assignment
- Lexical scoping

**Functions:**
- `(define (name params...) body)` - Function definition
- Function calls with arbitrary arguments
- Function names prefixed with `fn_` to avoid C++ keyword conflicts

**Control Flow:**
- `(if cond then else)` - Conditional expressions
- `(while cond body...)` - While loops
- `(for (var start end) body...)` - For loops (range-based)
- `(do expr1 expr2 ...)` - Sequential evaluation

**FFI (Foreign Function Interface):**
- `(c++ "native_code")` - Embed raw C++ code inline
- Example: `(c++ "std::abs(x)")` - Call C++ standard library functions

### Usage Example

```lisp
(do
  (define (factorial n)
    (if (= n 0)
        1
        (* n (factorial (- n 1)))))
  (factorial 5))
```

Transpiles to standalone C++ that outputs `120`.

### Implementation Notes

- Uses `int64_t` for all values (standalone, no tagged pointers)
- Functions are prefixed with `fn_` to avoid C++ keyword conflicts (e.g., `double` → `fn_double`)
- Variables are sanitized (hyphens → underscores, `?` → `_p`, `!` → `_bang`)
- Generated code includes `<cstdint>` and `<iostream>`
- Main function prints final result

## Important Implementation Details

### Memory Allocation
Uses `mmap()` with `MAP_ANONYMOUS` for zero-initialized memory. Memory is deallocated with `munmap()` in destructor.

### String Literals
Strings are stored in heap with:
- Word 0: Length (number of characters)
- Word 1+: Characters packed 8 per 64-bit word (little-endian byte order)

The compiler auto-allocates sequential variable slots for string storage.

### Stack Frame Layout (for functions)
```
[... caller stack ...]
[arg1]
[arg2]
[...argN]
[return_addr]        <- SP after CALL
```

Function prologue stores return address in temp variable, pops args into local variables, executes body, then restores return address and RET.

### Code Segment Protection
Code segment is write-protected during execution (checked in STORE opcode). This prevents self-modifying code bugs.

## Notes for Future Work

The TODO file mentions planned features:
- Transpiler missing structure definition
- Missing CALL/RET testing in test vm control
- Recursive function
- in lisp implementations of parser/compiler (currently header-only) with a c++ backend
- Hot-swappable microcode system
- heap, hp, sp are static should be configurable

When implementing Smalltalk features, refer to docs/SMALLTALK.md for the complete object model specification with classes, instances, and message sending patterns.
