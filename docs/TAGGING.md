# Tagged Pointers in Lisp

Demonstration of tagged pointer functions for distinguishing integers from object pointers (OOPs) using Lisp bitwise operations.

## Tag Scheme

Lower 3 bits of 64-bit values encode type:

```
Bits | Type    | Description
-----|---------|----------------------------------
000  | OOP     | Ordinary Object Pointer (heap address)
001  | INT     | Small integer (61-bit signed)
111  | SPECIAL | Special values (nil, true, false)
```

## Why 3 Bits?

Heap addresses are 8-byte aligned (divisible by 8), so their lower 3 bits are always `000`. We use these bits for type tags without losing address information.

## Integer Tagging

### Tag an Integer
Shift left by 3, OR with 1:
```lisp
(bit-or (bit-shl 42 3) 1)
; Result: 337
```

### Untag an Integer  
Arithmetic right shift by 3 (preserves sign):
```lisp
(bit-ashr 337 3)
; Result: 42
```

### How It Works
```
Original:  0000...00101010    (42)
Shifted:   0000...00101010000  (336)
Tagged:    0000...00101010001  (337)
                          ^^^-- TAG_INT (001)
```

### Round-trip Example
```lisp
(bit-ashr (bit-or (bit-shl 12345 3) 1) 3)
; Result: 12345
```

## OOP Tagging

### Tag an OOP
Address must be 8-byte aligned, tag is 0:
```lisp
(bit-or 16384 0)
; Result: 16384 (no change)
```

### Untag an OOP
Clear lower 3 bits:
```lisp
(bit-and 16384 (bit-xor -1 7))
; Result: 16384
```

### How It Works
```
Address:   0000...0100000000000000  (16384 = 0x4000)
                             ^^^--- Already 000 (8-byte aligned)
Tagged:    0000...0100000000000000  (same)
```

## Type Checking

### Check if Integer
```lisp
(= (bit-and 337 7) 1)
; Result: 1 (true - it's an int)
```

### Check if OOP
```lisp
(= (bit-and 16384 7) 0)
; Result: 1 (true - it's an OOP)
```

### Extract Tag Bits
```lisp
(bit-and 337 7)     ; Returns: 1 (TAG_INT)
(bit-and 16384 7)   ; Returns: 0 (TAG_OOP)
```

## Complete Example

```lisp
(do
  (print 42)
  (print (bit-or (bit-shl 42 3) 1))
  (print (bit-ashr 337 3))
  (print (= (bit-and 337 7) 1))
  999)

; Output:
; DEBUG: 42
; DEBUG: 337
; DEBUG: 42
; DEBUG: 1
; Result: 999
```

## Bitwise Operations

Available operations:

- `(bit-and a b)` - Bitwise AND
- `(bit-or a b)` - Bitwise OR
- `(bit-xor a b)` - Bitwise XOR
- `(bit-shl a n)` - Left shift by n bits
- `(bit-shr a n)` - Logical right shift
- `(bit-ashr a n)` - Arithmetic right shift (sign-extending)

## Common Patterns

### Tag Integer
```lisp
(bit-or (bit-shl value 3) 1)
```

### Untag Integer
```lisp
(bit-ashr tagged 3)
```

### Tag OOP
```lisp
(bit-or address 0)
```

### Untag OOP
```lisp
(bit-and tagged (bit-xor -1 7))
```

### Is Integer?
```lisp
(= (bit-and tagged 7) 1)
```

### Is OOP?
```lisp
(= (bit-and tagged 7) 0)
```

## Memory Layout

```
VM Memory (64K words = 512KB):
┌─────────────────────────────────┐
│ Code [0-16384)                  │ 128KB (bytecode)
├─────────────────────────────────┤
│ Heap [16384-65536)              │ 384KB (8-byte aligned)
│   All addresses end in 000      │
├─────────────────────────────────┤
│ Stack [grows down from 65536)   │
└─────────────────────────────────┘
```

Heap addresses: 16384, 16392, 16400, ... (all 8-byte aligned)

## Examples by Value

### Positive Integers
```lisp
(bit-or (bit-shl 0 3) 1)      ; 0 -> 1
(bit-or (bit-shl 42 3) 1)     ; 42 -> 337
(bit-or (bit-shl 99 3) 1)     ; 99 -> 793
(bit-or (bit-shl 12345 3) 1)  ; 12345 -> 98761
```

### Untagging
```lisp
(bit-ashr 1 3)      ; 1 -> 0
(bit-ashr 337 3)    ; 337 -> 42
(bit-ashr 793 3)    ; 793 -> 99
(bit-ashr 98761 3)  ; 98761 -> 12345
```

### OOP Addresses
```lisp
(bit-or 16384 0)  ; Heap start
(bit-or 32768 0)  ; Middle of heap
(bit-or 49152 0)  ; Upper heap
```

### Type Checks
```lisp
(= (bit-and 337 7) 1)    ; Is 337 an int? -> 1 (yes)
(= (bit-and 16384 7) 1)  ; Is 16384 an int? -> 0 (no)
(= (bit-and 16384 7) 0)  ; Is 16384 an OOP? -> 1 (yes)
(= (bit-and 337 7) 0)    ; Is 337 an OOP? -> 0 (no)
```

## Testing

Run the comprehensive test suite:

```bash
make simple
./simple_tagging_test
```

This demonstrates:
- Integer tagging/untagging
- OOP tagging/untagging  
- Type checking (is-int?, is-oop?)
- Round-trip conversions
- Tag extraction

## Integer Range

With 3 bits for tags, we have 61 bits for signed integers:

```
Max: 2^60 - 1 =  1,152,921,504,606,846,975
Min: -2^60    = -1,152,921,504,606,846,976
```

## Use Cases

1. **Garbage Collection** - Distinguish pointers from immediates during GC
2. **Type Checking** - Runtime type identification without separate type fields
3. **Memory Efficiency** - No overhead for type information
4. **Performance** - Type checks are single bitwise operations
5. **Safety** - Catch type errors at runtime

## Future Extensions

- Function pointers (tag 010)
- Characters (tag 011)  
- Symbols (tag 100)
- Boxed floats (tag 101)
- More special values (tag 111)
