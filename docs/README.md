# Minimal Stack VM with Lisp Compiler

A minimal 64-bit stack-based virtual machine with a unified flat memory model and a Lisp compiler.

## Architecture

### Unified Memory Model

The VM uses a single large `mmap`-allocated flat memory space (512KB / 64K 64-bit words):

```
Memory Layout (64-bit words):
┌─────────────────────────────────────────────┐
│ Code Segment        [0 ... 16384)           │  16K words (128KB)
│ - Executable bytecode                        │
│ - Read-only during execution                │
├─────────────────────────────────────────────┤
│ Heap                [16384 ... 65536)        │  ~49K words (384KB)
│ - Grows upward →                            │
│ - Dynamic allocation space                   │
├─────────────────────────────────────────────┤
│ Stack               [65536 ... 65536)        │  ~49K words (384KB)
│ - Grows downward ←                          │
│ - Function calls and local variables        │
└─────────────────────────────────────────────┘
```

### Registers

- **IP** (Instruction Pointer): Points to current instruction in code segment
- **SP** (Stack Pointer): Points to top of stack (grows downward from 65536)
- **BP** (Base Pointer): Frame pointer for function calls
- **HP** (Heap Pointer): Next free heap address (grows upward from 16384)

### Memory Safety

- Code segment is write-protected during execution
- Stack overflow detection (collision with heap)
- Stack underflow detection
- Bounds checking on all memory access
- Separate code/data spaces

## Instruction Set

### Stack Operations
- `PUSH <imm>` - Push 64-bit immediate value
- `POP` - Pop and discard top value
- `DUP` - Duplicate top of stack
- `SWAP` - Swap top two values

### Arithmetic
- `ADD`, `SUB`, `MUL`, `DIV`, `MOD`

### Comparison
- `EQ` (equal), `LT` (less than), `GT` (greater than)

### Control Flow
- `JMP <addr>` - Unconditional jump
- `JZ <addr>` - Jump if top of stack is zero
- `CALL <addr>` - Call function (saves return address and BP)
- `RET` - Return from function

### Memory Access
- `LOAD` - Load from memory address on stack
- `STORE` - Store value to memory address

### Debug
- `PRINT` - Print top of stack
- `HALT` - Stop execution

## Lisp Language

### Supported Forms

**Arithmetic**: `(+ a b ...)`, `(- a b ...)`, `(* a b ...)`, `(/ a b ...)`, `(% a b)`

**Comparison**: `(= a b)`, `(< a b)`, `(> a b)`

**Control Flow**: `(if condition then else)`

**Sequential Execution**: `(do expr1 expr2 ... exprN)` - Evaluates all expressions, returns last

**Debug**: `(print expr)` - Prints value and leaves it on stack

### Examples

```lisp
(+ 5 3)                           ; => 8
(* (+ 2 3) (- 10 4))             ; => 30
(if (< 5 10) 100 200)            ; => 100
(do (print 42) (+ 1 2 3))        ; Prints 42, returns 6
```

## Building and Running

```bash
make              # Compile
make run          # Run demo
make clean        # Clean build files
```

### Interactive REPL

```bash
./lisp_vm
```

Type expressions and see results immediately. Type `quit` or `exit` to exit.

## Implementation Details

### Memory Allocation

Uses `mmap` with `MAP_ANONYMOUS` for:
- Efficient memory management
- Zero-initialized memory
- Easy cleanup with `munmap`
- Potential for future features (memory protection, disk backing)

### Stack Model

The stack grows downward from `STACK_BASE` (65536). Each function call:
1. Pushes return address (IP)
2. Pushes old frame pointer (BP)
3. Sets new frame pointer (BP = SP)

Returns restore BP and IP from the stack.

### Compiler

The Lisp compiler generates bytecode in a single pass:
- Direct emission of instructions
- No intermediate representation
- Simple two-pass jump patching for conditionals
- Tail-recursive arithmetic operators

## Future Extensions

Potential additions:
- Heap allocation primitives (`alloc`, `free`)
- Garbage collection
- Variables and lexical scoping (`let`, `lambda`)
- Lists and cons cells
- More data types (strings, floats)
- File I/O
- JIT compilation
