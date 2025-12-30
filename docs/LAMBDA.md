# Lambda Functions in Lisp

The Lisp compiler now supports function definitions using a lambda-style syntax with `define`.

## Function Definition

**Syntax:** `(define (function-name param1 param2 ...) body)`

```lisp
(define (add x y) (+ x y))
(define (square x) (* x x))
(define (get-forty-two) 42)
```

## Function Calls

**Syntax:** `(function-name arg1 arg2 ...)`

```lisp
(add 10 20)           ; Returns: 30
(square 9)            ; Returns: 81
(get-forty-two)       ; Returns: 42
```

## Features

✓ Multiple parameters
✓ Zero parameters  
✓ Function composition (functions calling functions)
✓ Local variables in function body (using `define`, `let`)
✓ Control flow (`if`, `do`)
✓ All operators (arithmetic, bitwise, comparisons)

✗ Closures (functions don't capture outer scope)
✗ First-class functions (can't pass functions as values)
✗ Recursion (not yet implemented)

## Examples

### Simple Functions

```lisp
; One parameter
(do
  (define (add-five x) (+ x 5))
  (add-five 10))        ; Returns: 15

; Two parameters
(do
  (define (mul x y) (* x y))
  (mul 6 7))            ; Returns: 42

; Three parameters
(do
  (define (sum3 a b c) (+ a b c))
  (sum3 10 20 30))      ; Returns: 60
```

### Function Composition

```lisp
; Function calling another function
(do
  (define (add x y) (+ x y))
  (define (add-ten x) (add x 10))
  (add-ten 5))          ; Returns: 15

; Nested calls
(do
  (define (inc x) (+ x 1))
  (define (double x) (* x 2))
  (double (inc 5)))     ; Returns: 12
```

### Functions with Control Flow

```lisp
; Using if
(do
  (define (max x y)
    (if (> x y) x y))
  (max 10 20))          ; Returns: 20

; Using do for multiple statements
(do
  (define (compute x y)
    (do
      (define temp (* x 2))
      (define result (+ temp y))
      result))
  (compute 10 5))       ; Returns: 25
```

### Tagged Pointer Functions

```lisp
(do
  (define TAG_INT 1)
  
  ; Tag an integer
  (define (tag-int value)
    (bit-or (bit-shl value 3) TAG_INT))
  
  ; Untag an integer
  (define (untag-int tagged)
    (bit-ashr tagged 3))
  
  ; Check if tagged as integer
  (define (is-int? tagged)
    (= (bit-and tagged 7) TAG_INT))
  
  ; Use the functions
  (define original 99)
  (define tagged (tag-int original))
  (print tagged)            ; Prints: 793
  (define untagged (untag-int tagged))
  (print untagged)          ; Prints: 99
  (define check (is-int? tagged))
  (print check)             ; Prints: 1
  untagged)                 ; Returns: 99
```

### Iterative Calculations

```lisp
; Factorial (unrolled)
(do
  (define (fact5)
    (do
      (define result 1)
      (set result (* result 5))
      (set result (* result 4))
      (set result (* result 3))
      (set result (* result 2))
      result))
  (fact5))                  ; Returns: 120
```

## How It Works

### Calling Convention

1. **Caller** pushes arguments onto stack (in order)
2. **Caller** executes CALL (which pushes return address and jumps)
3. **Function** saves return address to a local variable
4. **Function** pops arguments into parameter variables
5. **Function** executes body (result left on stack)
6. **Function** restores return address and SWAPs it with result
7. **Function** executes RET (which pops result, pops ret addr, pushes result, jumps back)

### Memory Layout

Functions store:
- Return address in a hidden variable (`__ret_addr__`)
- Each parameter in its own variable
- Local variables defined with `define` or `let`

All function-local variables are allocated in the heap starting at address 16384.

### No Closures

Functions do NOT capture their defining environment. They only have access to:
- Their parameters
- Variables defined within the function
- Global variables

Example that WON'T work:
```lisp
(do
  (define x 10)
  (define (add-x y) (+ x y))  ; x is not accessible!
  (add-x 5))                  ; Error: Unbound symbol: x
```

To work around this, pass values as parameters:
```lisp
(do
  (define x 10)
  (define (add-values a b) (+ a b))
  (add-values x 5))           ; Returns: 15
```

## Limitations

**No Recursion**: Functions cannot call themselves (would need special handling to prevent infinite code generation)

**No Higher-Order Functions**: Functions cannot be passed as arguments or returned as values

**No Anonymous Lambda**: Must use `define` - standalone `(lambda (x) x)` is not supported

## Testing

Run the test suite:

```bash
make lambda
./test_lambda
```

This tests:
- Functions with 0, 1, 2, 3+ parameters
- Function composition and nesting
- Control flow in functions
- Local variables in functions
- Tagging system using functions
- Iterative calculations

## Implementation Details

### Function Storage

Functions are stored in a table mapping name to:
- Parameter list
- Body AST
- Code address (patched after compilation)

### Compilation Process

1. **First pass**: Collect function definitions
2. **Main code**: Compile main program with function calls
3. **Function compilation**: Compile all functions
4. **Patching**: Replace placeholder addresses with actual function addresses

### Stack Management

The VM uses a single stack that grows downward from address 65536. Function calls manipulate this stack by pushing/popping values. The compiler generates code to properly save and restore the stack state across calls.

## Future Enhancements

Possible additions:
- Tail call optimization
- Proper recursion support  
- Closures (capturing environment)
- First-class functions
- `apply` for variable arguments
- Multiple return values
