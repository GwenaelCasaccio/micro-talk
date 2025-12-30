# Variables and Scoping in Lisp

The Lisp compiler now supports proper variable binding with `define`, `let`, and `set`.

## Define

Define a variable in the current scope and bind it to a value. Returns the value.

**Syntax:** `(define name value)`

```lisp
(define x 42)           ; x = 42, returns 42
(define y (+ 10 20))    ; y = 30, returns 30
```

### Examples

```lisp
; Simple definition
(do
  (define x 10)
  (define y 20)
  (+ x y))              ; Result: 30

; Definition with expression
(do
  (define radius 5)
  (define area (* radius radius))
  area)                 ; Result: 25

; Define returns the value
(print (define result 99))  ; Prints 99
```

### Tagging Example with Define

```lisp
(do
  (define TAG_INT 1)
  (define value 42)
  (define tagged (bit-or (bit-shl value 3) TAG_INT))
  (print tagged)        ; Prints 337
  (define untagged (bit-ashr tagged 3))
  untagged)             ; Result: 42
```

## Set

Modify an existing variable. The variable must already be defined. Returns the new value.

**Syntax:** `(set name value)`

```lisp
(do
  (define x 10)
  (set x 20)            ; x becomes 20, returns 20
  x)                    ; Result: 20
```

### Examples

```lisp
; Simple mutation
(do
  (define counter 0)
  (set counter (+ counter 1))
  (set counter (+ counter 1))
  counter)              ; Result: 2

; Set returns the value
(do
  (define x 5)
  (print (set x 100)))  ; Prints 100

; Accumulator pattern
(do
  (define sum 0)
  (set sum (+ sum 10))
  (set sum (+ sum 20))
  (set sum (+ sum 30))
  sum)                  ; Result: 60
```

### Factorial-like Calculation

```lisp
(do
  (define n 5)
  (define result 1)
  (set result (* result n))
  (set n (- n 1))
  (set result (* result n))
  (set n (- n 1))
  (set result (* result n))
  (set n (- n 1))
  (set result (* result n))
  result)               ; Result: 120 (5 * 4 * 3 * 2)
```

## Let

Create local bindings in a new scope. The bindings are only visible within the let body.

**Syntax:** `(let ((name1 value1) (name2 value2) ...) body...)`

```lisp
(let ((x 10) (y 20))
  (+ x y))              ; Result: 30
```

### Features

- Creates a new scope
- Variables shadow outer definitions
- Multiple body expressions (returns last)
- Can be nested

### Examples

```lisp
; Simple let
(let ((x 42))
  x)                    ; Result: 42

; Multiple bindings
(let ((a 10) (b 20) (c 30))
  (+ a b c))            ; Result: 60

; Expressions in bindings
(let ((x 5) (y (* 3 4)))
  (+ x y))              ; Result: 17

; Multiple body expressions
(let ((x 5))
  (print x)             ; Prints 5
  (print (* x 2))       ; Prints 10
  (* x 3))              ; Returns 15
```

### Nested Let

```lisp
(let ((x 10))
  (let ((y 20))
    (let ((z 30))
      (+ x y z))))      ; Result: 60
```

### Variable Shadowing

```lisp
(do
  (define x 10)
  (let ((x 20))
    (print x)           ; Prints 20 (inner x)
    x)
  x)                    ; Result: 10 (outer x restored)
```

### Let with Tagging

```lisp
(let ((TAG_INT 1) (value 99))
  (let ((tagged (bit-or (bit-shl value 3) TAG_INT)))
    (let ((untagged (bit-ashr tagged 3)))
      untagged)))       ; Result: 99
```

## Scoping Rules

### Global Scope
Variables defined with `define` at the top level are global:

```lisp
(define global-var 100)
```

### Local Scope
Variables defined with `let` are local to that let block:

```lisp
(let ((local-var 42))
  local-var)
; local-var is not accessible here
```

### Scope Lookup
Variables are looked up from innermost to outermost scope:

```lisp
(define x 1)            ; Global: x = 1
(let ((x 2))            ; Outer let: x = 2
  (let ((x 3))          ; Inner let: x = 3
    (print x)           ; Prints 3
    x)
  (print x)             ; Prints 2
  x)
(print x)               ; Prints 1
```

### Set Finds Variables in Any Scope

```lisp
(define counter 0)
(let ((x 10))
  (set counter (+ counter x)))  ; Modifies outer counter
(print counter)         ; Prints 10
```

## Comparison with Other Forms

| Feature | define | set | let |
|---------|--------|-----|-----|
| Creates new binding | Yes | No | Yes |
| Modifies existing | No | Yes | No |
| Scope | Current | Any parent | New child |
| Returns | Value | Value | Last body expr |
| Can shadow | No | N/A | Yes |

## Complete Example: Tagged Pointer Library

```lisp
(do
  ; Define constants
  (define TAG_MASK 7)
  (define TAG_INT 1)
  (define TAG_OOP 0)
  
  ; Define tagging function
  (define tag-int-42
    (bit-or (bit-shl 42 3) TAG_INT))
  
  ; Use in let scope
  (let ((value 99))
    (let ((tagged (bit-or (bit-shl value 3) TAG_INT)))
      (let ((untagged (bit-ashr tagged 3)))
        (print untagged)    ; Prints 99
        
        ; Check type
        (let ((tag (bit-and tagged TAG_MASK)))
          (= tag TAG_INT))))) ; Returns 1 (true)
  
  ; Globals still accessible
  tag-int-42)               ; Returns 337
```

## Error Handling

### Undefined Variable

```lisp
x  ; Error: Unbound symbol: x
```

### Set on Undefined Variable

```lisp
(set x 10)  ; Error: set: undefined variable: x
```

### Redefinition in Same Scope

```lisp
(let ((x 10))
  (define x 20))  ; Error: Variable already defined in current scope: x
```

## Memory Layout

Variables are stored in the heap starting at address 16384:

```
┌──────────────────────┐
│ Code [0-16384)       │
├──────────────────────┤
│ Variables [16384+)   │  <- define/let allocate here
│ ...                  │
└──────────────────────┘
```

Each variable gets a unique 64-bit memory location. The compiler tracks variable names and their addresses in a scope stack.

## Best Practices

1. **Use `define` for top-level bindings:**
   ```lisp
   (define PI 314)
   (define max-value 1000)
   ```

2. **Use `let` for temporary calculations:**
   ```lisp
   (let ((temp (* x 2)))
     (+ temp y))
   ```

3. **Use `set` for mutation when needed:**
   ```lisp
   (define counter 0)
   (set counter (+ counter 1))
   ```

4. **Prefer `let` over `define` + `set` for locals:**
   ```lisp
   ; Good
   (let ((x 10) (y 20))
     (+ x y))
   
   ; Avoid
   (do
     (define x 10)
     (define y 20)
     (+ x y))
   ```

5. **Use meaningful names:**
   ```lisp
   (define tagged-value (bit-or (bit-shl value 3) TAG_INT))
   (define untagged-value (bit-ashr tagged-value 3))
   ```

## Testing

Run the test suite:

```bash
make vars
./test_variables
```

This tests:
- Simple definitions
- Variable usage and arithmetic
- Set operations and mutation
- Let with single and multiple bindings
- Nested let scopes
- Variable shadowing
- Complex examples with tagging
```