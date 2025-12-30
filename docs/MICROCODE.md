# Microcode System: Extending the VM with Lisp

The microcode system allows you to define new VM instructions using Lisp, making it possible to implement higher-level languages like Smalltalk without modifying the C++ VM code.

## Overview

**Microcode** is Lisp code that gets compiled into VM bytecode and registered as a new instruction with its own opcode (100-255). This allows you to:

1. **Extend the VM dynamically** - Add new instructions at runtime
2. **Build language primitives** - Implement Smalltalk, Ruby, Python operations
3. **Optimize common patterns** - Turn frequently-used Lisp idioms into single instructions
4. **Implement domain-specific operations** - Create specialized instructions for your application

## Defining Microcode

**Syntax:** `(defmicro name (param1 param2 ...) body)`

```lisp
(defmicro square (x) 
  (* x x))

(defmicro add3 (a b c)
  (+ a (+ b c)))
```

### How It Works

1. **Parse** the defmicro definition
2. **Compile** the body as a Lisp function
3. **Allocate** a unique opcode (100-255)
4. **Register** the compiled bytecode in the microcode table

The microcode system compiles your Lisp into efficient VM bytecode that can be called as a primitive operation.

## Example Microcodes

### Simple Operations

```lisp
; Square a number
(defmicro square (x)
  (* x x))

; Cube a number  
(defmicro cube (x)
  (* x (* x x)))

; Add three numbers
(defmicro add3 (a b c)
  (+ a (+ b c)))
```

### Tagged Integer Operations

```lisp
; Tag an integer (convert to tagged representation)
(defmicro tag-int (value)
  (bit-or (bit-shl value 3) 1))

; Untag an integer
(defmicro untag-int (tagged)
  (bit-ashr tagged 3))

; Check if value is a tagged integer
(defmicro is-tagged-int (obj)
  (= (bit-and obj 7) 1))

; Add two tagged integers (result is also tagged)
(defmicro tagged-add (a b)
  (do
    (define ua (bit-ashr a 3))
    (define ub (bit-ashr b 3))
    (define sum (+ ua ub))
    (bit-or (bit-shl sum 3) 1)))

; Multiply two tagged integers
(defmicro tagged-mul (a b)
  (do
    (define ua (bit-ashr a 3))
    (define ub (bit-ashr b 3))
    (define prod (* ua ub))
    (bit-or (bit-shl prod 3) 1)))
```

### Object Operations

```lisp
; Create an OOP (ordinary object pointer)
(defmicro make-oop (addr)
  (bit-or addr 0))

; Check if value is an OOP
(defmicro is-oop (obj)
  (= (bit-and obj 7) 0))

; Get the class of an object
(defmicro get-class (obj)
  (do
    (define addr (bit-and obj (bit-xor -1 7)))
    (define class-id addr)
    class-id))
```

## Smalltalk-like Primitives

The microcode system is perfect for implementing Smalltalk operations:

```lisp
; Create a new object of a given class
(defmicro new-object (class-id)
  (do
    (define TAG_OOP 0)
    (define obj-addr 16384)
    (bit-or obj-addr TAG_OOP)))

; Send a message to an object (simplified)
(defmicro send-msg (receiver selector)
  (do
    (define TAG_MASK 7)
    (define is-oop-check (= (bit-and receiver TAG_MASK) 0))
    (if is-oop-check receiver 0)))

; Array element access
(defmicro array-at (array index)
  (do
    (define base (bit-and array (bit-xor -1 7)))
    (define offset (+ base index))
    offset))
```

## Using the Microcode System

### Setup

```cpp
#include "microcode.hpp"

// Create microcode system and compiler
MicrocodeSystem microcode_sys;
MicrocodeCompiler compiler(microcode_sys);
```

### Define Microcodes

```cpp
// Define individual microcodes
compiler.compile_defmicro(
    "(defmicro square (x) (* x x))");

compiler.compile_defmicro(
    "(defmicro tag-int (value) "
    "  (bit-or (bit-shl value 3) 1))");

// Or load a library
SmalltalkMicrocode::define_smalltalk_primitives(compiler);
```

### Inspect Microcode Table

```cpp
// Print all defined microcodes
microcode_sys.print();

// List microcodes
for (const auto& name : microcode_sys.list()) {
    std::cout << name << std::endl;
}

// Check if an opcode is microcode
if (microcode_sys.is_microcode(100)) {
    const auto* mc = microcode_sys.get(100);
    std::cout << "Opcode 100: " << mc->name << std::endl;
}
```

## Opcode Allocation

- **Native opcodes**: 0-99 (built-in VM instructions)
- **Microcode opcodes**: 100-255 (user-defined)
- Maximum: 155 microcode instructions per VM

Opcodes are allocated sequentially starting at 100.

## Architecture

### MicrocodeSystem

Manages the microcode table:

```cpp
class MicrocodeSystem {
    struct Microcode {
        std::string name;
        uint8_t opcode;
        int param_count;
        std::vector<uint64_t> bytecode;
    };
    
    uint8_t define(name, param_count, bytecode);
    bool is_microcode(opcode);
    const Microcode* get(opcode);
    bool get_opcode(name, opcode&);
};
```

### MicrocodeCompiler

Compiles defmicro definitions:

```cpp
class MicrocodeCompiler {
    uint8_t compile_defmicro(source);
    std::vector<uint64_t> compile(source);
};
```

## Benefits

### 1. No VM Modification Needed

Add new instructions without touching C++:

```lisp
; Want a new instruction? Just define it!
(defmicro fibonacci (n)
  (do
    (define a 0)
    (define b 1)
    (for (i 0 n)
      (do
        (define temp b)
        (set b (+ a b))
        (set a temp)))
    a))
```

### 2. Language Implementation

Build higher-level language primitives:

```lisp
; Smalltalk-style message send
(defmicro smalltalk-send (receiver selector arg)
  (do
    (define class (get-class receiver))
    (define method (lookup-method class selector))
    (call-method method receiver arg)))

; Ruby-style block iteration
(defmicro ruby-times (n block)
  (for (i 0 n)
    (call-block block i)))
```

### 3. Performance

Microcode is compiled once and reused:

```lisp
; Instead of writing this every time:
(bit-ashr (bit-or (bit-shl value 3) 1) 3)

; Define once, use everywhere:
(defmicro tag-untag-cycle (value)
  (bit-ashr (bit-or (bit-shl value 3) 1) 3))
```

### 4. Domain-Specific Extensions

Create specialized instructions for your domain:

```lisp
; Graphics operations
(defmicro make-point (x y)
  (+ (bit-shl x 32) y))

(defmicro point-x (point)
  (bit-shr point 32))

; Network operations  
(defmicro pack-ipv4 (a b c d)
  (+ (bit-shl a 24)
     (bit-shl b 16)
     (bit-shl c 8)
     d))
```

## Example: Building a Smalltalk VM

```cpp
MicrocodeSystem microcode_sys;
MicrocodeCompiler compiler(microcode_sys);

// Object model
compiler.compile_defmicro(
    "(defmicro new-object (class-id size) "
    "  (do "
    "    (define addr (allocate size)) "
    "    (store-class addr class-id) "
    "    (tag-oop addr)))");

// Message send
compiler.compile_defmicro(
    "(defmicro send (receiver selector) "
    "  (do "
    "    (define class (get-class receiver)) "
    "    (define method (lookup-method class selector)) "
    "    (if method "
    "        (call-method method receiver) "
    "        (send receiver 'doesNotUnderstand))))");

// Integer operations (tagged)
compiler.compile_defmicro(
    "(defmicro int-add (a b) "
    "  (do "
    "    (if (and (is-int a) (is-int b)) "
    "        (do "
    "          (define result (+ (untag a) (untag b))) "
    "          (tag-int result)) "
    "        (send a '+ b))))");

// Class creation
compiler.compile_defmicro(
    "(defmicro define-class (name superclass methods) "
    "  (do "
    "    (define class (new-object MetaClass 3)) "
    "    (set-name class name) "
    "    (set-superclass class superclass) "
    "    (set-methods class methods) "
    "    class))");
```

## Limitations

**Current Implementation:**
- Microcodes are compiled to regular Lisp functions
- No direct opcode execution yet (future enhancement)
- Calling overhead same as regular functions

**Planned Enhancements:**
- Direct opcode dispatch in VM execute loop
- Inline microcode into caller for hot paths
- Optimized parameter passing for microcodes

## Comparison with Functions

| Feature | Regular Function | Microcode |
|---------|-----------------|-----------|
| Definition | `(define (name ...) ...)` | `(defmicro name (...) ...)` |
| Opcode | CALL instruction | Dedicated opcode |
| Overhead | Call/return frame setup | (Currently same, future: lower) |
| Purpose | General code reuse | VM primitives |
| Use case | Application logic | Language implementation |

## Testing

```bash
make micro
./test_microcode
```

Tests demonstrate:
- Basic arithmetic microcodes
- Tagged integer operations
- Type checking primitives
- Smalltalk-like object operations

## Future Work

### 1. Direct Opcode Execution

Add microcode dispatch to VM execute loop:

```cpp
case Opcode::MICROCODE_BASE + 0:  // square
    execute_microcode(100);
    break;
```

### 2. Inline Compilation

Inline hot microcodes into caller:

```lisp
; Instead of CALL 100
; Directly emit: MUL instructions
```

### 3. JIT Compilation

Compile microcodes to native code for even better performance.

### 4. Microcode Libraries

Standard libraries of microcodes:

- `smalltalk.lisp` - Smalltalk primitives
- `ruby.lisp` - Ruby operations  
- `python.lisp` - Python operations
- `graphics.lisp` - Graphics primitives

## Conclusion

The microcode system transforms Lisp from a scripting language into a **metaprogramming platform** for VM extension. You can now:

1. **Implement new languages** on top of the VM
2. **Define domain-specific primitives** without C++ knowledge
3. **Optimize hot paths** by turning Lisp idioms into instructions
4. **Experiment rapidly** with new VM features

This makes the VM truly extensible and suitable for implementing sophisticated systems like Smalltalk, Self, or custom domain-specific languages.
